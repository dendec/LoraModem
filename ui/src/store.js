import Vue from 'vue'
import Vuex from 'vuex'
import LoRaModem from './modem.js'

Vue.use(Vuex);

export const store = new Vuex.Store({
    state: {
        modem: undefined,
        connected: false,
        history: [],
        stat: {},
    },
    mutations: {
        createModem(state, address) {
            state.modem = new LoRaModem(address);
            state.modem.addEventListener("onReceive", (e) => {
                this.commit("addHistoryMessage", {...e.detail, isReceived: true});
            });
            state.modem.addEventListener("onDisconnect", () => state.connected = false);
            state.modem.addEventListener("onConnect", () => state.connected = true);
        },
        removeModem(state) {
            state.modem.disconnect();
            state.modem = undefined;
            state.connected = false;
            state.history = [];
            state.stat = {};
        },
        setStat(state, stat) {
            state.stat = {tx: Number(stat.tx), rx: Number(stat.rx)};
        },
        updateTX(state, delta) {
            let tx = state.stat.tx + Number(delta);
            state.stat = {...state.stat, tx};
        },
        updateRX(state, delta) {
            let rx = state.stat.rx + Number(delta);
            state.stat = {...state.stat, rx};
        },
        addHistoryMessage(state, message) {
            let getMessageLength = (message) => {
                if (!message)
                    return 0
                if (message.binary)
                    return message.binary.size;
                if (message.text)
                    return message.text.length;
                return 0;
            }
            state.history.push({ ...message, timestamp: new Date() });
            if (message.isReceived) {
                this.commit("updateRX", getMessageLength(message));
            } else {
                this.commit("updateTX", getMessageLength(message));
            }
        },
        addHistoryFileMessage(state, message) {
            state.history.push({ ...message, timestamp: new Date()});
        },
        updateLastHistoryMessage(state, message) {
            let index = state.history.length - 1;
            let item = state.history[index];
            state.history.splice(index, 1, {...item, ...message});
        }
    }
});