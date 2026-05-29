<script lang="ts" setup>
import { onMounted, onUnmounted, ref } from "vue";
import { useRoute } from "vue-router";

// 声明 imouPlayer 类型
declare const imouPlayer: any;

// 扩展 window 对象类型
declare global {
  interface Window {
    player: any;
  }
}

const route = useRoute();
interface IPlayer {
  play: Function;
  stop: Function;
  pause: Function;
  start: Function;
  capture: Function;
  startTalk: Function;
  stopTalk: Function;
  volume: Function;
  fullScreen: Function;
  exitFullScreen: Function;
  startRecord: Function;
  stopRecord: Function;
  destroy: Function;
  status: {
    playing: boolean;
  };
}

let player: IPlayer;

const destroy = () => {
  player.destroy();
  player = null!;
};

const init = () => {
  if (player) {
    destroy();
  }
  player = new imouPlayer({
    id: "imou-player",
    width: window.document.documentElement.clientWidth,
    height: 211,
    deviceId: deviceInfo.value.deviceId,
    channelId: deviceInfo.value.channelId,
    token: deviceInfo.value.kitToken,
    // 1-Live 直播; 2-Playback 录播
    type: deviceInfo.value.type,
    // Live 0-HD 高清; 1-SD 标清
    streamId: deviceInfo.value.streamId,
    // 录播 云录像 cloud 本地录像 localRecord 默认 云录像
    // Playback, cloud-Cloud Video; localRecord-Local Video. Default Cloud Video
    recordType: deviceInfo.value.recordType,
    muted: false,
    templateMode: "mobile",
    code: deviceInfo.value.code,
    handleError: (err: any) => {
      console.error("handleError", err);
    },
  });
  window.player = player;
};

const deviceInfo = ref({
  deviceId: "",
  channelId: "",
  kitToken: "",
  code: "",
  streamId: "",
  recordType: "",
  type: "",
});

onMounted(() => {
  const list = JSON.parse(localStorage.getItem("list") || "[]");
  const id = route.query.id;
  const info = list.find((item: any) => item.id == id);
  deviceInfo.value = info;
  init();

  // 页面处于播放状态时，监听页面可见性变化
  if (player?.status?.playing) {
    // 监听页面可见性变化（包括home键、切换应用等）
    document.addEventListener("visibilitychange", () => {
      if (document.hidden) {
        // 页面进入后台（包括点击home键）
        if (player) {
          player.pause();
        }
      } else {
        // 页面重新可见
        if (player) {
          player.start();
        }
      }
    });
  }
});
onUnmounted(() => {
  console.log("ImouPlayer Unmounted");

  if (player) {
    destroy();
  }
});
</script>

<template>
  <div class="imou-player">
    <div id="imou-player" style="width: 100%; height: 211px; background-color: #000"></div>
  </div>
</template>

<style scoped>
.imou-player {
  padding: 0;
}
</style>
