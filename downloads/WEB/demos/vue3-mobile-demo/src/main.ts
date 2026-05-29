import { createApp } from "vue";
import "./style.less";
import App from "./App.vue";
import router from "./router";
import { Button, NavBar, Popup, Picker, Form, Field, CellGroup, SwipeCell, Dialog } from "vant";
import "vant/lib/index.css";

const app = createApp(App);
app.use(router);
app.mount("#app");
app.use(Button);
app.use(NavBar);
app.use(Form);
app.use(Field);
app.use(CellGroup);
app.use(Popup);
app.use(Picker);
app.use(SwipeCell);
app.use(Dialog);
