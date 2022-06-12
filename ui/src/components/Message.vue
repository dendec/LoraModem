<template>
    <div class="message">
        <div v-if="message.text" class="message-box" :class="{received: message.isReceived}">
            <div class="message-text">{{ message.text }}</div>
            <div class="message-time">{{ formatDate(message.timestamp) }}</div>
            <div class="progress" v-show="message.filename">
                <span style="width: 30%"></span>
            </div>
        </div>
        <div v-else class="message-box" :class="{received: message.isReceived}">
            <div class="message-text">
                <p>File: <a :href=dataURL :download=message.filename>{{ message.filename }}</a></p>
                <p>Size: {{ message.size }}</p>
                <p v-if="hash">MD5: {{ hash }}</p>
            </div>
            <div class="message-time">{{ formatDate(message.timestamp) }}</div>
            <div class="progress" v-if="message.isReceived">
                <span v-bind:style="{ width: fileReceiveProgress }"></span>
            </div>
            <div class="progress" v-else>
                <span v-bind:style="{ width: fileSendProgress }"></span>
            </div>
        </div>
    </div>
</template>

<script>
import CryptoJS from "crypto-js";
export default {
    name: 'Message',
    props: ['message'],
    data: () => {
        return {
            dataURL: undefined,
            intervalId: undefined,
            hash:  undefined,
            fileReceiveProgress: "0%",
        }
    },
    computed: {
        fileSendProgress() {
            if (this.message.progress) {
                return `${this.message.progress}%`;
            }
            return `0%`;
        }
    },
    methods: {
        formatDate(date) {
            return date.toLocaleString();
        },
        updateMessage() {
            if (this.message.file) {
                let reader = new FileReader();
                reader.onload = e => {
                    this.dataURL = e.target.result;
                    this.fileReceiveProgress = "100%";
                    let bstr = window.atob(e.target.result.split(',')[1]);
                    this.hash = CryptoJS.MD5(bstr).toString(CryptoJS.enc.Hex);
                };
                reader.readAsDataURL(this.message.file);
                if (this.message.isReceived) {
                    clearInterval(this.intervalId);
                }
            }
            if (this.message.chunk) {
                let totalChunks = Math.ceil(this.message.size / 250) + 1
                this.fileReceiveProgress = this.message.chunk / totalChunks * 100 + "%";
            }
        }
    },
    created: function() {
        if (this.message.filename) {
            if (this.message.isReceived) {
                this.intervalId = setInterval(this.updateMessage, 100);
            } else {
                this.updateMessage()
            }
        }
    }
}
</script>

<style scoped>
.message-time {
    font-size: 12px;
    padding: 5px;
    text-align: right;
}
.message-text {
    white-space: pre-line;
}
.message-box {
    padding: 10px;
    margin-bottom: 0.5%;
    border-radius: var(--border);
    width: 80%;
    overflow-wrap: break-word;
    background-color: var(--message-color);
}
.message-box.received {
    margin-left: 18.5%;
}
</style>
