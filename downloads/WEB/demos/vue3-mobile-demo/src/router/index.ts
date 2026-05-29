import { createRouter, createWebHistory } from "vue-router";
import type { RouteRecordRaw } from "vue-router";

const routes: RouteRecordRaw[] = [
  {
    path: "/",
    name: "Home",
    component: () => import("../views/Home.vue"),
  },
  {
    path: "/player",
    name: "Player",
    component: () => import("../views/Player.vue"),
  },
  {
    path: "/edit",
    name: "Edit",
    component: () => import("../views/Edit.vue"),
  },
];
const router = createRouter({
  history: process.env.NODE_ENV === "production" ? createWebHistory("/imou-player/mobile/") : createWebHistory(),
  routes,
});

export default router;
