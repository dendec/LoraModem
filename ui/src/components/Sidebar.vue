<template>
    <aside>
        <div v-show="!connected">
            No connection
        </div>
        <div v-show="connected">
            <a title="Disconnect" v-if="!hasLocalServer">
                <button  @click="disconnect()">
                    <svg width="20" height="20" viewBox="0 0 1024 1024">
                        <path d="M832.6 191.4c-84.6-84.6-221.5-84.6-306 0l-96.9 96.9 51 51 96.9-96.9c53.8-53.8 144.6-59.5 204 0 59.5 59.5 53.8 150.2 0 204l-96.9 96.9 51.1 51.1 96.9-96.9c84.4-84.6 84.4-221.5-.1-306.1zM446.5 781.6c-53.8 53.8-144.6 59.5-204 0-59.5-59.5-53.8-150.2 0-204l96.9-96.9-51.1-51.1-96.9 96.9c-84.6 84.6-84.6 221.5 0 306s221.5 84.6 306 0l96.9-96.9-51-51-96.8 97zM260.3 209.4a8.03 8.03 0 0 0-11.3 0L209.4 249a8.03 8.03 0 0 0 0 11.3l554.4 554.4c3.1 3.1 8.2 3.1 11.3 0l39.6-39.6c3.1-3.1 3.1-8.2 0-11.3L260.3 209.4z"/>
                    </svg>
                </button>
            </a>
            <a title="Update">
                <button @click="updateValues()" :disabled="isUpdating">
                    <svg width="20px" height="20px" viewBox="0 0 489.935 489.935">
                        <path d="M278.235,33.267c-116.7,0-211.6,95-211.6,211.7v0.7l-41.9-63.1c-4.1-6.2-12.5-7.9-18.7-3.8c-6.2,4.1-7.9,12.5-3.8,18.7
                            l60.8,91.5c2.2,3.3,5.7,5.4,9.6,5.9c0.6,0.1,1.1,0.1,1.7,0.1c3.3,0,6.5-1.2,9-3.5l84.5-76.1c5.5-5,6-13.5,1-19.1
                            c-5-5.5-13.5-6-19.1-1l-56.1,50.7v-1c0-101.9,82.8-184.7,184.6-184.7s184.7,82.8,184.7,184.7s-82.8,184.7-184.6,184.7
                            c-49.3,0-95.7-19.2-130.5-54.1c-5.3-5.3-13.8-5.3-19.1,0c-5.3,5.3-5.3,13.8,0,19.1c40,40,93.1,62,149.6,62
                            c116.6,0,211.6-94.9,211.6-211.7S394.935,33.267,278.235,33.267z"/>
                    </svg>
                </button>
            </a>
            <a title="Reboot">
                <button @click="reboot()">
                    <svg viewBox="0 0 489.888 489.888" width="20px" height="20px">
                        <path d="M25.383,290.5c-7.2-77.5,25.9-147.7,80.8-192.3c21.4-17.4,53.4-2.5,53.4,25l0,0c0,10.1-4.8,19.4-12.6,25.7
                            c-38.9,31.7-62.3,81.7-56.6,136.9c7.4,71.9,65,130.1,136.8,138.1c93.7,10.5,173.3-62.9,173.3-154.5c0-48.6-22.5-92.1-57.6-120.6
                            c-7.8-6.3-12.5-15.6-12.5-25.6l0,0c0-27.2,31.5-42.6,52.7-25.6c50.2,40.5,82.4,102.4,82.4,171.8c0,126.9-107.8,229.2-236.7,219.9
                            C122.183,481.8,35.283,396.9,25.383,290.5z M244.883,0c-18,0-32.5,14.6-32.5,32.5v149.7c0,18,14.6,32.5,32.5,32.5
                            s32.5-14.6,32.5-32.5V32.5C277.383,14.6,262.883,0,244.883,0z"/>
                    </svg>
                </button>
            </a>
            <a title="Reset to defaults">
                <button @click="reset()">
                    <svg width="20px" height="20px" viewBox="0 0 15 15">
                        <path d="M4.85355 2.14645C5.04882 2.34171 5.04882 2.65829 4.85355 2.85355L3.70711 4H9C11.4853 4 13.5 6.01472 13.5 8.5C13.5 10.9853 11.4853 13 9 13H5C4.72386 13 4.5 12.7761 4.5 12.5C4.5 12.2239 4.72386 12 5 12H9C10.933 12 12.5 10.433 12.5 8.5C12.5 6.567 10.933 5 9 5H3.70711L4.85355 6.14645C5.04882 6.34171 5.04882 6.65829 4.85355 6.85355C4.65829 7.04882 4.34171 7.04882 4.14645 6.85355L2.14645 4.85355C1.95118 4.65829 1.95118 4.34171 2.14645 4.14645L4.14645 2.14645C4.34171 1.95118 4.65829 1.95118 4.85355 2.14645Z"/>
                    </svg>
                </button>
            </a>
            <h3>Modem</h3>
            <ul>
                <li><span>tx:</span> <span>{{ stat.tx }}</span></li>
                <li><span>rx:</span> <span>{{ stat.rx }}</span></li>
                <li><span>address:</span> <span>{{ config.addr }}</span></li>
                <li><span>bandwidth:</span><select v-model="config.bw" @change="e => changeSetting('bandwidth', e)">
                        <option>7</option>
                        <option>10</option>
                        <option>15</option>
                        <option>20</option>
                        <option>31</option>
                        <option>41</option>
                        <option>62</option>
                        <option>125</option>
                        <option>250</option>
                        <option>500</option>
                    </select> MHz</li>
                <li><span>frequency:</span>
                    <input v-model="config.freq" @change="e => changeSetting('frequency', e)" id="freq-input" type="number" step="0.1" min="410.0" max="525.0"> MHz
                </li>
                <li><span>power:</span>
                    <span>
                        <input v-model="config.pow" @change="e => changeSetting('power', e)" id="pow-range" type="range" min="2" max="17">{{ config.pow }} dBm
                    </span>
                </li>
                <li><span>spreading factor:</span><select v-model="config.sf" @change="e => changeSetting('spreadingFactor', e)">
                        <option>6</option>
                        <option>7</option>
                        <option>8</option>
                        <option>9</option>
                        <option>10</option>
                        <option>11</option>
                        <option>12</option>
                    </select></li>
                <li><span>code rate:</span><select v-model="config.cr" @change="e => changeSetting('rate', e)">
                        <option>5</option>
                        <option>6</option>
                        <option>7</option>
                        <option>8</option>
                    </select> /4</li>
                <li><span>accept files:</span> <input v-model="acceptFiles" type="checkbox" @change="acceptFilesChange()"></li>
            </ul>
            <!--<h3>WiFi</h3>
             <ul>
                <li><span>Mode:</span>
                    <select v-model="wifi.mode">
                        <option value="0">OFF</option>
                        <option value="1">AP</option>
                        <option value="2">STA</option>
                    </select>
                </li>
                <li><span>SSID:</span>
                    <input type="text" v-model="wifi.ssid">
                </li>
                <li><span>Pass:</span>
                    <input type="password" minlength="8" v-model="wifi.password">
                </li>
            </ul>
            <form>
                <label>Mode:
                    <select v-model="wifi.mode">
                        <option value="0">OFF</option>
                        <option value="1">AP</option>
                        <option value="2">STA</option>
                    </select>
                </label>
                <label>SSID:
                    <input type="text" v-model="wifi.ssid">
                </label>
                <label>Pass:
                    <input type="password" minlength="8" v-model="wifi.password">
                </label>
                <button type="submit" id="apply-button" @click="applyWifiSettings">Apply</button>
            </form>-->
            <h3>Nodes</h3>
            <ul id="nodes-list">
                <li v-for="node in nodes" :key="node.id" v-bind:item="node" 
                    @click="selectedNode == node.id ? selectedNode = '' : selectedNode = node.id" 
                    :class="{selected: selectedNode == node.id}">
                    <span>{{ node.addr }}</span> <span>{{ node.rssi }}</span>
                    <span v-show="Number(node.period) < Number(node.age)">?</span>
                </li>
            </ul>
        </div>
    </aside>
</template>

<script>
export default {
    name: 'Sidebar',
    data: () => {
        return {
            config: {},
            wifi: {},
            nodes: [],
            hasLocalServer: true,
            isUpdating: false,
            acceptFiles: false,
            selectedNode: ""
        }
    },
    computed: {
        modem() {
            return this.$store.state.modem;
        },
        stat() {
            return this.$store.state.stat;
        },
        connected() {
            return this.$store.state.connected;
        },
    },
    methods: {
        updateValues() {
            this.isUpdating = true;
            this.modem.command.config().then(config => {
                this.config = config;
                this.modem.command.stat().then(stat => {
                    this.$store.commit("setStat", stat);  
                    this.modem.command.scan().then(nodes => {
                        this.nodes = nodes;
                        this.modem.command.wifi().then(wifi => {
                            this.wifi = wifi;
                            this.isUpdating = false;
                        }, () => this.isUpdating = false)
                    }, () => this.isUpdating = false)
                }, () => this.isUpdating = false)
            }, () => this.isUpdating = false)
        },
        disconnect() {
            this.$store.commit("removeModem");  
        },
        acceptFilesChange() {
            this.modem.fileSender.setReceiveEnabled(this.acceptFiles);
            if (this.acceptFiles) {
                this.modem.fileSender.addEventListener("startFileReceive", e => {
                    console.log(e.detail)
                    this.$store.commit("addHistoryFileMessage", {...e.detail, isReceived: true});
                });
                this.modem.fileSender.addEventListener("proceedFileReceive", e => {
                    console.log(e.detail)
                    this.$store.commit("updateLastHistoryMessage", e.detail);
                });
                this.modem.fileSender.addEventListener("finishFileReceive", e => {
                    console.log(e.detail)
                    this.$store.commit("updateLastHistoryMessage", e.detail);
                });
            }
        },
        reboot() {
            this.modem.command.restart()
        },
        reset() {
            this.modem.command.reset().then(() => {
                this.modem.command.config().then(config => {
                    this.config = config;
                })
            })
        },
        changeSetting(setting, event) {
           this.modem.command[setting](event.target.value)
        },
        applyWifiSettings() {
            
        }
    },
    created: function() {
        this.$watch("connected", () => {
            if (this.connected) {
                this.updateValues();
            }
        });
        this.$watch("config", () => {
            document.title = `LoraModem ${this.config.addr}`;
        });
        this.hasLocalServer = process.env.VUE_APP_EMBEDDED === "true";
    }
}
</script>

<style scoped>
button {
    margin-right: 10px;
}
aside {
    width: 20%;
    background-color: var(--sidebar-color);
    padding: 30px 10px 30px 10px;
    overflow: auto;
}
ul {
    margin: 10px;
    list-style-type: none;
    text-align: left;
}
ul > li {
    padding-bottom: 5px;
    display: table;
    width: 100%;
}
li > span {
    display: table-cell;
    width: 50%;
}
form {
    margin: 10px;
    text-align: left;
}
form > label {
    display: block;
    margin-bottom: 5px;
}
#pow-range {
    padding: 0px;
    cursor: pointer;
    margin-right: 5px;
    width: 45px;
}
#freq-input {
    width: 60px;
    padding: 2px;
}
#nodes-list > li {
    padding: 10px;
    border-radius: var(--border-radius);
    cursor: pointer;
}
#nodes-list > li:hover {
    background-color: var(--button-color);
    color: var(--font-color-light);
}
#nodes-list > li.selected {
    background-color: var(--button-color);
    color: var(--font-color-light);
}
#apply-button {
    width: 40px;
}
h3 {
    margin-top: 10px;
}
</style>
