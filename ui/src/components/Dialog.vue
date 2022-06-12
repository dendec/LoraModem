<template>
    <div class="dialog">
        <div v-if="!hasLocalServer" v-show="modem == undefined || connected == false" class="connect">
            <input v-model="address" type="text" v-on:keyup.enter="connect()"/>
            <a title="Connect">
                <button @click="connect()" id="connect-button">
                    <svg width="20px" height="20px" viewBox="0 0 15 15">
                        <path
                            fill-rule="evenodd"
                            clip-rule="evenodd"
                            d="M4.5 1C4.22386 1 4 1.22386 4 1.5C4 1.77614 4.22386 2 4.5 2H12V13H4.5C4.22386 13 4 13.2239 4 13.5C4 13.7761 4.22386 14 4.5 14H12C12.5523 14 13 13.5523 13 13V2C13 1.44772 12.5523 1 12 1H4.5ZM6.60355 4.89645C6.40829 4.70118 6.09171 4.70118 5.89645 4.89645C5.70118 5.09171 5.70118 5.40829 5.89645 5.60355L7.29289 7H0.5C0.223858 7 0 7.22386 0 7.5C0 7.77614 0.223858 8 0.5 8H7.29289L5.89645 9.39645C5.70118 9.59171 5.70118 9.90829 5.89645 10.1036C6.09171 10.2988 6.40829 10.2988 6.60355 10.1036L8.85355 7.85355C9.04882 7.65829 9.04882 7.34171 8.85355 7.14645L6.60355 4.89645Z"
                        />
                    </svg>
                </button>
            </a>
        </div>
        <div v-show="modem != undefined && connected == true" >
            <div id="history">
                <message v-for="historyItem in history" :key="historyItem.id" v-bind:message="historyItem"/>
            </div>
            <div class="relative textarea-container">
                <textarea v-model="message" 
                    v-on:keyup.ctrl.enter="send()" 
                    v-on:keyup.delete="clearFile()" 
                    v-on:input="checkInput()"
                    :readonly="file"></textarea>
                <a title="Send message">
                    <button @click="send()" id="send-button">
                        <svg width="20px" height="20px" viewBox="0 0 32 32">
                            <path
                                d="M 0,32 3.219592,17.408533 22.311837,15.793548 22.315137,15.597086 3.258776,14.508221 0.42449,0 32,15.563788 Z"/>
                        </svg>
                    </button>
                </a>
                <a title="Attach file">
                    <label id="attach-button">
                        <svg width="20px" height="20px" viewBox="0 0 1000 1000">
                            <path 
                                d="M678.3,233.1H745V744c0,68.1-23.6,126.2-70.9,174.1C626.8,966,569.1,990,501,990c-68.1,0-126.2-24-174.1-71.9c-48-48-71.9-106-71.9-174.1V189.3c0-48.7,17.7-90.7,53.2-126.2C343.6,27.7,385.6,10,434.3,10c48.7,0,90.4,17.7,125.1,53.2c34.8,35.4,52.1,77.5,52.1,126.2v467.1c0,30.6-10.8,56.6-32.3,78.2c-21.6,21.6-47.6,32.3-78.2,32.3c-30.6,0-57-10.8-79.2-32.3c-22.2-21.6-33.4-47.6-33.4-78.2V233.1h66.7v423.3c0,12.5,4.5,22.9,13.6,31.3c9,8.3,19.8,12.5,32.3,12.5c12.5,0,22.9-4.2,31.3-12.5c8.3-8.3,12.5-18.8,12.5-31.3V189.3c0-30.6-10.8-57-32.3-79.2c-21.6-22.2-47.6-33.4-78.2-33.4c-30.6,0-57,11.1-79.2,33.4c-22.2,22.2-33.4,48.6-33.4,79.2V744c0,48.7,17.7,90.7,53.2,126.2c35.4,35.4,77.5,53.2,126.2,53.2c48.7,0,90.4-17.7,125.1-53.2c34.8-35.4,52.1-77.5,52.1-126.2V233.1L678.3,233.1z"/>
                        </svg>
                        <input id="file-upload" type="file" @change="attachFile"/>
                    </label>
                </a>
                <div class="progress">
                    <span v-bind:style="{ width: messageProgress }" v-show="!file"></span>
                </div>
            </div>
        </div>
    </div>
</template>

<script>
import CryptoJS from "crypto-js";
import Message from "./Message.vue";
const MESSAGE_SIZE = 250

export default {
    name: 'Dialog',
    components: {
        Message
    },
    data: () => {
        return {
            address: "192.168.0.114",
            message: "",
            file: null,
            hasLocalServer: true,
        }
    },
    computed: {
        modem() {
            return this.$store.state.modem;
        },
        connected() {
            return this.$store.state.connected;
            //return true
        },
        history() {
            return this.$store.state.history;
        },
        messageProgress() {
            let percent = this.getMessageSize(this.message) / MESSAGE_SIZE * 100;
            return `${percent}%`;
        }
    },
    methods: {
        connect() {
            this.$store.commit("createModem", `ws://${this.address}/ws`);
        },
        send() {
            if (this.modem) {
                let text = this.message
                if (this.file) {
                    this.modem.fileSender.sendFile(this.file);
                    this.$store.commit("addHistoryFileMessage", {filename: this.file.name, size: this.file.size, file: this.file, isReceived: false});
                    this.modem.fileSender.addEventListener("proceedFileUpload", e => {
                        this.$store.commit("updateLastHistoryMessage", e.detail);
                    });
                } else if (text) {
                    this.modem.send(text);
                    this.$store.commit("addHistoryMessage", {text, isReceived: false});
                }
                this.message = "";
                this.file = null;
            }
        },
        checkInput() {
            let message = this.message;
            let size = this.getMessageSize(message);
            while (size > MESSAGE_SIZE) {
                try {
                    size = this.getMessageSize(message.slice(0, -1));
                    message = message.slice(0, -1);
                } catch (error) {
                    size = this.getMessageSize(message.slice(0, -2));
                    message = message.slice(0, -2);
                }
            }
            this.message = message;
        },
        attachFile(event) {
            if (event.target.files.length > 0) {
                this.file = event.target.files[0];
                this.message = `File: ${this.file.name}\nSize: ${this.file.size}`
            }
        },
        clearFile() {
            this.file = null;
            this.message = "";
        },
        getMessageSize(message) {
            return CryptoJS.enc.Utf8.parse(message).sigBytes
        }
    },
    created: function() {
        this.hasLocalServer = process.env.VUE_APP_EMBEDDED === "true"
        if (this.hasLocalServer) {
            this.address = window.location.hostname;
            this.connect();
        }
    }
}
</script>

<style scoped>
#history {
    height: 80vh;
    overflow-x: hidden;
    padding: 0.5%;
    margin-bottom: 10px;
}
.dialog {
    width: 80%;
}
.connect {
    margin: auto;
    margin-top: 100px;
    position: relative;
    width: 170px;
}
.message-box {
    text-align: left;
    padding: 10px;
    border-radius: var(--border);
    background-color: var(--message-color);
}
input {
    border-radius: var(--border);
    height: 29px;
    padding-left: 5px;
    position: absolute;
    top: 0px;
    left: 0px;
    border: 1px solid transparent;
}
input:focus{
    outline: none;
    border: 1px solid var(--font-color-dark);
}
#connect-button {
    position: absolute;
    right: 0px;
}
textarea {
    resize: none;
    height: 60px;
    width: 700px;
    border-radius: var(--border);
    border: 1px solid transparent;
    padding: 8px;
}
textarea:focus{
    outline: none;
    border: 1px solid var(--font-color-dark);
}
.textarea-container {
    width: 720px;
    margin: auto;
}
#send-button {
    position: absolute;
    bottom: 10px;
    right: 2px;
}
input[type="file"] {
    display: none;
}
#attach-button {
    position: absolute;
    bottom: 10px;
    right: 35px;
    background-color: var(--button-color);
    border: none;
    color: var(--font-color-light);
    border-radius: var(--border);
    cursor: pointer;
    border: 1px solid transparent;
    width: 30px;
    height: 29px;
}
#attach-button:hover > svg {
    fill: var(--button-color);
}
#attach-button > svg {
    fill: var(--font-color-light);
    margin-top: 4px;
}
#attach-button:hover {
  background-color: var(--font-color-light);
  color: var(--button-color);
  border: 1px solid var(--button-color);
}
.progress {
  height: 5px;
  overflow: hidden;
  position: relative;
}
.progress span {
  height: 100%;
  display: block;
  border-radius: var(--border);
  background-color: var(--button-color);
}
</style>
