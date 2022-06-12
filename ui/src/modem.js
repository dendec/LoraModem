import checksum from './checksum.js'

const SYMBOL_SOH = 0x01;
const SYMBOL_STX = 0x02;
const SYMBOL_EOT = 0x04;
const SYMBOL_ACK = 0x06;
const SYMBOL_NACK = 0x15;
const SYMBOL_CPMEOF = 0x1A;
const SYMBOL_C = 0x43;

class FileSender extends EventTarget {

    static protocol = Object.freeze({
        OPTIMAL_NO_CHECKS: {
            dataSize: 250,
            packetSize: 250,
            hasNamePacket: true,
            hasCounter: false,
            hasCRC: false,
            hasChecksum: false,
            initialChar: SYMBOL_NACK,
            hasPrefix: false,
            prefixChar: undefined,
            hasPadding: false,
            paddingChar: undefined,
        },
        Y_MODEM_1k: {
            dataSize: 1024,
            packetSize: 1024 + 5,
            hasNamePacket: true,
            hasCounter: true,
            hasReverseCounter: true,
            hasCRC: true,
            hasChecksum: false,
            hasPrefix: true,
            prefixChar: SYMBOL_STX,
            hasPadding: true,
            paddingChar: SYMBOL_CPMEOF,
        },
        Y_MODEM: {
            dataSize: 128,
            hasNamePacket: true,
            hasCounter: true,
            hasChecksum: true,
            initialChar: SYMBOL_C,
            prefixChar: SYMBOL_SOH,
            paddingChar: SYMBOL_CPMEOF,
        },
        Y_MODEM_OPTIMAL: {
            dataSize: 245,
            hasNamePacket: true,
            hasCounter: true,
            hasChecksum: true,
            initialChar: SYMBOL_C,
            prefixChar: SYMBOL_SOH,
            paddingChar: SYMBOL_CPMEOF,
        }
    })

    _defaultProtocol = FileSender.protocol.Y_MODEM_OPTIMAL;
    _socket;
    _readyToReceiveTimer;
    _receiveTimeout;
    _onMessage;
    _receivedChunks = [];
    _protocol;
    _name;

    constructor(socket) {
        super();
        this._socket = socket;
    }
    
    setReceiveEnabled(isReceiveEnabled, protocol) {
        if (isReceiveEnabled) {
            if (!protocol) {
                this._protocol = this._defaultProtocol;
            } else {
                this._protocol = protocol;
            }
            if (!this._onMessage) {
                this._onMessage = this._socket.onmessage;
            }
            this._socket.onmessage = this._receive;
            this._readyToReceiveTimer = setInterval(() => {
                this._socket.send(new Uint8Array([this._protocol.initialChar]));
            }, 3000);
        } else {
            if (this._readyToReceiveTimer) {
                clearTimeout(this._readyToReceiveTimer);
                this._socket.onmessage = this._onMessage;
            }
        }
    }

    sendFile(file, protocol) {
        if (!protocol) {
            this._protocol = this._defaultProtocol;
        } else {
            this._protocol = protocol;
        }
        let name = file.name;
        let size = file.size;
        console.log(`File ${name}: ${size} bytes`);
        let reader = new FileReader();
        reader.onload = () => this._send(name, size, reader.result);
        reader.readAsArrayBuffer(file);
    }

    _send(name, size, buffer) {
        this._onMessage = this._socket.onmessage;
        let totalPackets = Math.ceil(size / this._protocol.dataSize);
        let promise = Promise.resolve();
        if (this._protocol.hasNamePacket) { // send file name
            console.log(`Sent name packet`);
            const packet = this.createPacket(this._namePacketPayload(name, size), 0)
            promise = promise.then(() => this.sendWithAcknowledge(packet.buffer, 0));
        }
        for (let chunkNumber = 0; chunkNumber < totalPackets; chunkNumber ++) {
            promise = promise
                .then(result => {
                    let {n, err, count} = result;
                    let data = buffer.slice(
                        chunkNumber * this._protocol.dataSize, 
                        (chunkNumber + 1) * this._protocol.dataSize);
                    let packet = this.createPacket(new Uint8Array(data), n+1)
                    if (err) {
                        console.log(`Packet ${n} error: ${err}`);
                        return Promise.reject(err);
                    } else {
                        console.log(`Sent packet ${n} from ${totalPackets} attempt ${count}`);
                    }
                    this.dispatchEvent(new CustomEvent("proceedFileUpload", {detail: {chunk: n+1, progress: (n+1) / totalPackets * 100}}));
                    return this.sendWithAcknowledge(packet, n+1);
                });
        }
        promise
            .then(result => {
                console.log("File transmission finishing");
                this.sendWithAcknowledge(new Uint8Array([SYMBOL_EOT]).buffer, result.n + 1)
            })
            .finally(() => {
                console.log("File transmission finished");
                this._socket.onmessage = this._onMessage;
            });
    }

    _finishReceive = () => {
        this._readyToReceiveTimer = setInterval(() => {
            this._socket.send(new Uint8Array([this._protocol.initialChar]));
        }, 3000);
        this._receivedChunks = [];
        this._name = null;
    }

    _receive = (event) => {
        if (this._readyToReceiveTimer) {
            clearTimeout(this._readyToReceiveTimer);
        }
        clearTimeout(this._receiveTimeout);
        this._receiveTimeout = setTimeout(() => {
            console.log(`File ${this._name} receive timeout`);
            this._finishReceive();
        }, 10000);
        event.data.arrayBuffer().then(receivedPacket => {
            if (receivedPacket.byteLength == 1) {
                let value = new Uint8Array(receivedPacket)[0];
                if (value == SYMBOL_EOT) {
                    console.log(`File ${this._name} received`);
                    this.dispatchEvent(new CustomEvent("finishFileReceive", {detail: {file: this._getFile()}}));
                    this._socket.send(new Uint8Array([SYMBOL_ACK]));
                    this._finishReceive();
                } else {
                    this._socket.send(new Uint8Array([SYMBOL_NACK]));
                }
            } else {
                let packet = this.parsePacket(receivedPacket, this._receivedChunks.length);
                if (packet.isSuccess) {
                    if (this._receivedChunks.length == 0 && packet.counter == 0 && !this._name) {
                        let namePacket = this.decodeNamePacket(packet.chunk);
                        this._name = namePacket.filename;
                        this.dispatchEvent(new CustomEvent("startFileReceive", {detail: namePacket}));
                    } else {
                        if (this._protocol.hasCounter) {
                            let chunkIndex = Math.floor(this._receivedChunks.length/255)*255 + packet.counter
                            this._receivedChunks[chunkIndex] = packet.chunk;
                        } else {
                            this._receivedChunks.push(packet.chunk);
                        }
                        this.dispatchEvent(new CustomEvent("proceedFileReceive", {detail: {chunk: this._receivedChunks.length}}));
                    }
                    this._socket.send(new Uint8Array([SYMBOL_ACK]));
                } else {
                    this._socket.send(new Uint8Array([SYMBOL_NACK]));
                }
            }
        });
    }

    _getFile() {
        if (this._protocol.hasNamePacket) {
            this._receivedChunks.shift();
        }
        let lastChunkIndex = this._receivedChunks.length - 1;
        let lastChunk = this._receivedChunks[lastChunkIndex];
        this._receivedChunks[lastChunkIndex] = this._removeTrailingPadding(lastChunk, this._protocol.paddingChar);
        return new File(this._receivedChunks, this._name);
    }

    _namePacketPayload(name, size) {
        let data = new Uint8Array(this._protocol.dataSize);
        let filename = new TextEncoder().encode(name);
        let filesize = new TextEncoder().encode(size);
        if (filename.length + filesize.length + 1 > this._protocol.dataSize) {
            data.set(filename.slice(0, this._protocol.dataSize));
            data.set([0, ...filesize], this._protocol.dataSize - filesize.length - 1);
        } else {
            data.set(filename);
            data.set([0, ...filesize], filename.length);
        }
        return data;
    }

    createPacket(data, packetNum, isCRC = true) {
        let packet = new Uint8Array();
        if (data.length < this._protocol.dataSize) {
            let dataWithPadding = new Uint8Array(this._protocol.dataSize)
            dataWithPadding.fill(this._protocol.paddingChar);
            dataWithPadding.set(data, 0);
            data = dataWithPadding;
        }
        if (this._protocol.prefixChar !== undefined) {
            packet = new Uint8Array([this._protocol.prefixChar]);
        }
        if (this._protocol.hasCounter) {
            packet = Uint8Array.of(...packet, packetNum, 255 - packetNum);
        }
        packet = Uint8Array.of(...packet, ...data);
        if (this._protocol.hasChecksum) {
            if (isCRC) {
                packet = Uint8Array.of(...packet, ...checksum.crc16(data));
            } else {
                packet = Uint8Array.of(...packet, ...checksum.xor(data));
            }
        }
        return packet;
    }

    _removeTrailingPadding(byteArray, padding = 0) {
        let reversed = new Uint8Array(byteArray).reverse();
        return reversed.subarray(reversed.findIndex(i => i !== padding)).reverse();
    }

    parsePacket(packet) {
        function toHexString(byteArray) {
            return Array.from(byteArray, function(byte) {
                return ('0' + (byte & 0xFF).toString(16)).slice(-2);
            }).join(' ')
        }
        let packetArray = this._removeTrailingPadding(new Uint8Array(packet));
        console.log(toHexString(packetArray));
        //console.log(new TextDecoder().decode(packetArray));
        let isSuccess = true;
        let offset = 0;
        if (this._protocol.prefixChar !== undefined) {
            let prefix = packetArray[offset++];
            isSuccess &= (prefix === this._protocol.prefixChar);
            if (!isSuccess) {
                console.log("Invalid prefix");
                return { isSuccess }
            }
        }
        let counter = 0;
        if (this._protocol.hasCounter) {
            counter = packetArray[offset++];
            let reverseCounter = packetArray[offset++];
            isSuccess &= ((counter ^ reverseCounter) == 0xFF);
            if (!isSuccess) {
                console.log("Invalid counter");
                return { isSuccess }
            }
        }
        let chunk = new Uint8Array(packet.slice(offset));
        if (this._protocol.hasChecksum) {
            let isValid = false;
            let chunkWithChecksum = chunk;
            // assume CRC
            chunk = chunkWithChecksum.slice(0, chunkWithChecksum.length - 2);
            isValid |= checksum.equals(chunkWithChecksum.slice(-2), checksum.crc16(chunk));
            if (!isValid) { // assume XOR
                chunk = chunkWithChecksum.slice(0, chunkWithChecksum.length - 1);
                isValid |= checksum.equals(chunkWithChecksum.slice(-1), checksum.xor(chunk));
            }
            isSuccess &= isValid;
            if (!isSuccess) {
                console.log("Invalid hash");
                return { isSuccess }
            }
        }
        return { isSuccess, counter, chunk }
    }

    decodeNamePacket(namePacket) {
        if (this._protocol.hasNamePacket) {
            let [filename, size] = new TextDecoder().decode(namePacket).split('\0')
            console.log(`name ${filename}, size ${size}`)
            return {filename, size}
        } else {
            return {filename: new Date().toISOString()};
        }
    }

    sendWithAcknowledge(payload, n, timeoutMs, count) {
        if (!timeoutMs) {
            timeoutMs = 3000;
        }
        if (!count) {
            count = 0;
        }
        return new Promise((resolve, reject) => {
            let timer = setTimeout(() => {
                reject({n, err: "timeout", count});
            }, timeoutMs);
            this._socket.onmessage = (event) => {
                clearTimeout(timer);
                event.data.arrayBuffer().then(buffer => {
                    let ack = new Int8Array(buffer)[0];
                    if (ack == SYMBOL_ACK) {
                        //console.log(`n=${n} count=${count} ack`);
                        resolve({n, count});
                    } else {
                        //console.log(`n=${n} count=${count} nack`);
                        reject({n, err: "nack", count});
                    }
                });
            }
            this._socket.send(payload);
            console.log(`sending n=${n} count=${count}`);
        }).catch(result => {
            let {n, err, count} = result;
            console.log(`error err=${err} count=${count}`);
            if (count < 3) {
                return this.sendWithAcknowledge(payload, n, timeoutMs, count + 1);
            } else {
                return Promise.reject(err);
            }
        });
    }
}

class LoRaModem extends EventTarget {
    connected = false;
    _url;
    _socket;
    fileSender;

    constructor(websocketUrl) {
        super();
        if (websocketUrl) {
            this._url = websocketUrl;
        } else {
            this._url = `ws://${window.location.hostname}/ws`;
        }
        this._connect();
        this.fileSender = new FileSender(this._socket);
    }

    _connect() {
        this._socket = new WebSocket(this._url);
        this._socket.onopen = () => {
            console.log(`Connected to ${this._url}`);
            this.connected = true;
            this.dispatchEvent(new CustomEvent("onConnect"));
        };
        this._socket.onclose = (event) => {
            this.connected = false;
            console.log('Socket is closed. Reconnect will be attempted in 1 second.', event.reason);
            this.dispatchEvent(new CustomEvent("onDisconnect"));
            setTimeout(() => {
                this._connect();
            }, 1000);
        };
        this._socket.onerror = (err) => {
            console.error('Socket encountered error: ', err.message, 'Closing socket');
            this._socket.close();
        };
        this.listen();
    }

    disconnect() {
        if (this._socket) {
            this._socket.onclose = undefined;
            this._socket.close();
        } 
    }

    listen() {
        this._socket.onmessage = (event) => {
            event.data.text().then(text => {
                if (text != '\x15' && text != 'C') { // file transmission enabled
                    this.dispatchEvent(new CustomEvent("onReceive", {detail: {binary: event.data, text: text}}));
                }
            })
        };
    }

    send(payload) {
        this._socket.send(payload);
    }

    get command() {
        return (() => {
            function parseCsv(response, keys) {
                let result = [];
                let lines = response.trim().split('\n');
                if (!keys) {
                    keys = lines.shift().split(',');
                }
                lines.forEach(line => {
                    let values = line.split(',');
                    let resultItem = {};
                    keys.forEach((key, i) => {
                        resultItem[key] = values[i].trim()
                    });
                    result.push(resultItem);
                });
                return result;
            }

            function parseCsvObject(response, keys) {
                let arr = parseCsv(response, keys);
                if (arr.length > 0) {
                    return arr[0];
                }
                return {};
            }

            function parseConf(response) {
                return parseCsvObject(response, ['addr', 'freq', 'bw', 'sf', 'cr', 'pow']);
            }

            function parseStat(response) {
                return parseCsvObject(response, ['tx', 'rx']);
            }

            function parseScan(response) {
                return parseCsv(response, ['addr', 'rssi', 'age']);
            }

            function parseWiFi(response) {
                return parseCsvObject(response, ['mode', 'channel', 'ssid', 'password']);
            }

            function makeWifi(wifi) {
                if (wifi.mode == 0) {
                    return '0,,,'
                } else {
                    return `${wifi.mode},1,${wifi.ssid},${wifi.password}`
                }
            }

            return {
                AT: () => this.sendCommand('AT'),
                reset: () => this.sendCommand('ATZ'),
                restart: () => this.sendCommand('AT+RST'),
                config: () => this.sendCommand('AT+CONF').then(parseConf),
                address: () => this.sendCommand('AT+ADDR'),
                frequency: (value) => (value) ? this.sendCommand(`AT+FREQ=${value}`) : this.sendCommand('AT+FREQ'),
                bandwidth: (value) => (value) ? this.sendCommand(`AT+BW=${value}`) : this.sendCommand('AT+BANDWIDTH'),
                spreadingFactor: (value) => (value) ? this.sendCommand(`AT+SF=${value}`) : this.sendCommand('AT+SFACTOR'),
                power: (value) => (value) ? this.sendCommand(`AT+POW=${value}`) : this.sendCommand('AT+POWER'),
                scan: () => this.sendCommand('AT+SCAN').then(parseScan),
                stat: () => this.sendCommand('AT+STAT').then(parseStat),
                wifi: (value) => (value) ? this.sendCommand(`AT+WIFI=${makeWifi(value)}`) : this.sendCommand('AT+WIFI').them(parseWiFi),
                rate: (value) => (value) ? this.sendCommand(`AT+RATE=${value}`) : this.sendCommand('AT+RATE'),
            }
        })();
    }

    sendCommand(command, timeoutMs) {
        if (!command.endsWith('\n')) { 
            command = command + '\n';
        }
        if (!timeoutMs) {
            timeoutMs = 10000;
        }
        return new Promise((resolve, reject) => {
            let timer = setTimeout(() => {
                reject();
                this.listen();
            }, timeoutMs);
            this._socket.onmessage = (event) => {
                clearTimeout(timer);
                event.data.text().then(resolve);
                this.listen();
            }
            this.send(command);
        });
    }

}

export {
    LoRaModem as default,
    FileSender
}
