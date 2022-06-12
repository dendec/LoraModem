<template>
  <div id="app">
    <Sidebar/>
    <Dialog/>
  </div>
</template>

<script>
import Dialog from './components/Dialog.vue'
import Sidebar from './components/Sidebar.vue'

export default {
  name: 'App',
  components: {
    Dialog,
    Sidebar
  },
  created(){
    document.title = "LoraModem"
  }
}
</script>

<style>
:root {
  --body-color: #c3cee1;
  --font-color-dark: #393539;
  --message-color: #ebf0f7;
  --button-color: #475069;
  --font-color-light: #c3cee1;
  --sidebar-color: #8e9fbb;
  --border: 3px;

}

*, html, body {
   margin: 0;
   padding: 0;
   outline: 0;
}

body {
  background-color: var(--body-color);
  margin-top: -60px;
}

#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: var(--font-color-dark);
  margin-top: 60px;
  height: 100vh;
  overflow: hidden;
}

.wrapper {
  max-width: 1400px;
  margin: auto;
  padding: 20px;
}

.relative {
  position: relative;
}

button {
  background-color: var(--button-color);
  border: none;
  color: var(--font-color-light);
  border-radius: var(--border);
  cursor: pointer;
  border: 1px solid transparent;
  width: 30px;
  height: 30px;
}

button > svg {
  fill: var(--font-color-light);
}

button:hover > svg {
  fill: var(--button-color);
}

button:hover {
  background-color: var(--font-color-light);
  color: var(--button-color);
  border: 1px solid var(--button-color);
}

#app {
  display: flex;
}
</style>
