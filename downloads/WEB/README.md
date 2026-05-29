## ImouPlayer - 组件说明

### 体验产品

**在线体验产品功能，之后可基于轻应用 Demo 进行二次开发。体验地址:**

- PC端： [点击体验轻应用](https://open.imou.com/imou-player/index.html)
- 移动端：[点击体验轻应用](https://open.imou.com/imou-player/mobile/)

**<font color=red>凡2024年12月31日以后的版本接入请关注</font> [1.2 依赖库引入](#WasmLib)**

### 1. 基础用法

#### 1.1 imou-player.js 引入

```html
// 需同步引入css样式文件
<link href="./imou-player.css" rel="stylesheet" />
<script src="./imou-player.js"></script>
```

```html
例：
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, intial-scale=1, maxium-scale=1, user-scalable=no" />
    <title>imouPlayer</title>
    <link href="./imou-player.css" rel="stylesheet" />
    <script src="./imou-player.js"></script>
  </head>
  <body>
    <!-- 在页面中提供一个空的div标签用于生成轻应用播放器元素 -->
    <div id="root"></div>
  </body>
</html>
```
<h3 id="WasmLib"></h3>
#### 1.2 依赖库引入【重要且必须】 [详见多线程模式配置](#section1) 

**将 WasmLib 引入到自己项目的 public 下，**
**imouPlayer初始化支持WasmLib访问路径配置，参数:WasmLibPath   根据自己项目可配置相对路径(默认)、绝对路径**

#### 1.3 初始化 imouPlayer

```javascript
// 添加DOM容器
const player = new imouPlayer({
  id: "root",
  width: 800,
  height: 400,
  // 设备序列号
  deviceId: "7H0B18XXXXXXXX",
  channelId: 0,
  token: "Kt_hz00e4c3XXXXXXXXXXX",
  // 1 直播  2 录播
  type: 1,
  // 直播 0 高清  1 标清  默认
  streamId: 0,
  // 录播 云录像 cloud 本地录像 localRecord 默认 云录像
  recordType: "cloud",
  WasmLibPath: "", // 默认相对路径，绝对路径可配置"/"，具体路径根据项目public文件路径调整
  // 如果设备设置了自定义音视频加密秘钥，则输入此秘钥；
  // 如果设备只设置了设备密码，则输入设备密码；其他情况默认设备序列号。
  code: "xxxxxx",
});
```

#### 1.4 imouPlayer 初始化参数说明

| 参数名      | 类型     | 默认值 | 必填 | 说明                                                                                                                                                                              |
| ----------- | -------- | ------ | ---- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| id          | String   | /      | Yes  | 容器挂载 Dom 的 Id |
| width       | Number   | /      | Yes  | 容器宽度 |
| height      | Number   | /      | Yes  | 容器高度 |
| deviceId    | String   | /      | Yes  | 设备序列号 |
| channelId   | String   | /      | Yes  | 设备通道号 |
| token       | String   | /      | Yes  | 轻应用播放 token |
| type        | String   | /      | Yes  | 播放类型： 1：直播（实时预览）； 2：录像回放；|
| recordType  | String   | /      | No   | type=2 的情况下默认云录像回放；若 recordType=localRecord 则本地录像回放 |
| beginTime   | String   | /      | No   | 回放开始时间   YYYY-MM-DD HH:mm:ss|
| endTime     | String   | /      | No   | 回放结束时间   YYYY-MM-DD HH:mm:ss|
| streamId    | String   | 0      | No   | 清晰度： 0：高清；1：标清  |
| muted       | Boolean  | false  | No   | 静音 |
| code        | String   | /      | No   | 设备视频解密秘钥，如果设备开启了视频加密，则必填。<br> 如果设备设置了自定义音视频加密秘钥，则填此秘钥；<br> 如果设备只设置了设备密码，则填设备密码；<br> 其他情况默认设备序列号。 |
| handleError | Function | /      | No   | 播放错误回调，详见[1.4.1 handleError](#handleError)  |
| handleCallBack| Function | /      | No   | 播放事件回调，详见[1.4.2 handleCallBack](#handleCallBack)  |
| videoTalk   | Boolean  | false  | No   |视频对讲 |
| controls   | Boolean  | true  | No   |控件显示true/隐藏false |
| controlsConfig   | Array  | ["play", "volume", "talk", "capture", "videoRecord", "ptz", "resolution", "fullPageScreen", "fullScreen", "speed", "recordChange", "recordTimeLine","calendar","digitalZoom"]  | No   |控件配置，配置项参考[1.4.3controlsConfig配置](#controlsConfig) |
| controlsSize   | Number  | PC端：35；移动端：32  | No   |控件大小设置 |
| controlsColor   | String  | #ffffff  | No   |控件颜色设置 |
| controlsActiveColor   | String  | #f18d00  | No   |控件高亮颜色设置 |
| templateMode   | String  | pc  | No   |UI模板配置<br/> PC端：pc   移动端：mobile |
| title   | String  | 设备序列号-通道号  | No   |标题内容 |
| titleColor   | String  | #ffffff  | No   |标题颜色 |
| WasmLibPath   | String  | ""  | No   |WasmLib资源访问路径，默认相对路径，绝对路径可以配置："/"，具体路径根据项目public文件路径调整 |
| dpr   | Number  | 0  | No   | V1.3.0以后版本支持，消除监控画面清晰度超过当前终端的显示分辨率时产生的锯齿。默认为0，按照原始码流分辨率渲染。 |



<h3 id="handleError"></h3>
##### 1.4.1 handleError 播放错误回调

| errCode | errMsg                   |
| ------- | ------------------------ |
| 1001    | 解密失败，请重新输入秘钥 |
| 1002    | 设备响应异常，请检查后重试 |
| 2001    | 获取对讲地址失败，请检查设备 |
| 2002    | 对讲连接建立失败, 已存在语音对讲流源 |
| 2003    | 设备不支持视频对讲 |
| 2004    | 麦克风已被占用 或 无法获取麦克风权限 |
| 2005    | 摄像头已被占用 或 无法获取摄像头权限 |
| 2006    | 对讲失败，请稍后重试 |
| 2007    | 视频对讲，设备端已挂断 |
| 2008    | 对讲忙线 |
| 2009    | 对讲已关闭 |

<h3 id="handleCallBack"></h3>
##### 1.4.2 handleCallBack 播放事件回调

| type | desc                   |
| ------- | ------------------------ |
| playStart    | 开始播放 |
| talkStart    | 开始对讲 |
| talkEnd      | 对讲结束 |

<h3 id="controlsConfig"></h3>
##### 1.4.3 controlsConfig配置  
> controls 为true的情况下才生效

| type | desc                   |
| ------- | ------------------------ |
| play    | 播放/暂停 |
| volume    | 声音开关 |
| talk    | 对讲 |
| resolution | 清晰度切换 |
| capture    | 抓图 |
| ptz    | 云台 |
| videoRecord    | 屏幕录制 |
| fullPageScreen    | 网页全屏 |
| fullScreen    | 全屏 |
| speed    | 倍速 |
| recordChange    | 云录像、本地录像切换 |
| recordTimeLine    | 录像时间轴 |
| calendar    | 日历 |
| digitalZoom    | 电子变焦 |


#### 1.5 getKitToken 接口协议说明

##### 详细说明

获取轻应用播放 token；

> 注：
>
> 1、开发者私有云调用乐橙开放平台接口的协议的域名地址和签名计算说明请见：[开发规范](/pages/c20750/)
>
> 2、kitToken 有效期为 2 小时，建议开发者在自身服务中针对 kitToken 缓存一小时处理，而不用每次调用开放平台接口获取。

##### 请求地址

https://openapi.lechange.cn/openapi/getKitToken

##### 传入参数说明

| 参数列表  | 说明               | 参数类型 | 是否必填 | 默认值 | 合法值                                                                |
| --------- | ------------------ | -------- | -------- | ------ | --------------------------------------------------------------------- |
| token     | 管理员 accessToken | String   | 是       |        | accessToken 获取协议请参考：[accessToken](/pages/fef620/)             |
| deviceId  | 设备序列号         | String   | 是       |        |                                                                       |
| channelId | 设备通道号         | String   | 是       |        |                                                                       |
| type      | 默认传 0           | String   | 是       |        | 0：所有权限；1：实时预览；2：录像回放（云录像+本地录像）；6：云台转动 |

##### 样例输入

```json
{
  "system": {
    "ver": "1.0",
    "appId": "lc0****426c",
    "sign": "812e419af25bf773d8959d7dd82dc259",
    "time": 1626313677,
    "nonce": "1686f6f4-dec7-486e-93e3-6b2740577259"
  },
  "id": "0f935cb0-5ddc-44d4-9058-6b5fc928839e",
  "params": {
    "token": "At_000085fa18f0319046199b2138c04e54",
    "deviceId": "7B06****Z396E9",
    "channelId": "0",
    "type": "0"
  }
}
```

### 2. imouPlayer 方法集合

| 方法名         | 功能描述                                                  | 使用示例                                                         |
| -------------- | --------------------------------------------------------- | ---------------------------------------------------------------- |
| play           | 开始播放                                                  | player.play()                                                    |
| pause          | 暂停播放                                                  | player.pause()                                                   |
| start          | 暂停后开始播放                                            | player.start()                                                   |
| destroy        | 结束播放同时销毁 DOM                                      | player.destroy()                                                 |
| capture        | 截图                                                      | player.capture()                                                 |
| startTalk      | 开始对讲                                                  | player.startTalk() // 语音对讲 <br> player.startTalk("video") // 视频对讲   |
| stopTalk       | 结束对讲                                                  | player.stopTalk()                                                |
| volume         | 设置音量（0/1）                                           | player.volume(0)  // 关闭音量 <br> player.volume(1)  // 打开音量 |
| fullScreen     | 全屏                                                      | player.fullScreen()                                              |
| exitFullScreen | 退出全屏                                                  | player.exitFullScreen()                                          |
| startRecord    | 开始屏幕录制                                              | player.startRecord()                                             |
| stopRecord     | 结束屏幕录制                                              | player.stopRecord()                                              |
| setSpeed       | 录像倍速播放（0.5/1/2/4/8/16/32），本地录像不支持 32 倍速 | player.setSpeed(2)                                               |
| answerVideoTalk       | 接收到设备发起可视对讲，进入接听/拒接状态 | player.answerVideoTalk()                                   |
| zoomIn       | 电子变焦-放大 | player.zoomIn()                                   |
| zoomOut       | 电子变焦-缩小 | player.zoomOut()                                   |
| resetZoom       | 电子变焦-重置 | player.resetZoom()                                   |

#### 2.1 相关示例代码

```js
const text = e.target.innerText;
switch (text) {
  case "播放":
    player.play();
    break;
  case "暂停":
    player.pause();
    break;
  case "暂停后开始播放":
    player.start();
    break;
  case "截图":
    player.capture();
    break;
  case "全屏":
    player.fullScreen();
    break;
  case "退出全屏":
    player.exitFullScreen();
    break;
  case "声音-打开":
    player.volume(1);
    break;
  case "声音-关闭":
    player.volume(0);
    break;
  case "语音对讲-开始对讲":
    player.startTalk();
    break;
  case "语音对讲-结束对讲":
    player.stopTalk();
    break;
  case "2倍速":
    player.setSpeed(2);
    break;
  default:
    break;
}
```

<h3 id="section1"></h3>
### 3. 多线程模式配置

 **imouPlayer JSSDK 支持 web 端无插件播放 H264/H265 编码的直播/录像。使用浏览器共享线程方案（SharedArrayBuffer），支持同时创建多个线程协同处理解码播放。默认使用多线程解码（谷歌浏览器 >= 91 版本；火狐浏览器 >= 97 版本；Edge 浏览器 >= 91 版本使用），不满足则使用单线程；**

 **多线程模式在相同环境下有更好的性能表现，同时需要对服务进行跨域嵌入策略配置**

> **您可以通过浏览 demo 快速上手： [资源下载](/pages/a69478/) 中 轻应用直播套件内 demos/**

- **将 WasmLib 引入到自己项目的 public 下（重要且必须）**

- 配置前端服务，在响应头中添加 Cross-Origin-Opener-Policy 和 Cross-Origin-Embedder-Policy 配置：

  - node 服务

  ```javascript
  response.setHeader("Cross-Origin-Opener-Policy", "same-origin");
  response.setHeader("Cross-Origin-Embedder-Policy", "require-corp");
  ```

  - nginx 服务

  ```javascript
  add_header Cross-Origin-Opener-Policy "same-origin";
  add_header Cross-Origin-Embedder-Policy "require-corp";
  ```

### 常见问题

1. 直播/录像播放、对讲等情况建立 websocket 连接时，返回的信息是 404？
   
> 返回 404 可能存在流源超时无效的情况，实时录像、对讲、直播、录像下载的流地址，要避免提前请求，需要使用时，再触发接口获取流地址，避免流源超时无效，避免同时请求不同流源导致流源混乱。
   
2. 发起对讲，设备端无声音？
   
> 有可能是通道音频格式是 ACC，但是 RTSP 信令返回的是 L16，目前通过 NVR 和 通道 IPC 对讲，不支持 L16。
   
3. 页面放置多个播放器页面，页面卡顿问题
   
> 因需对码流进行解密，且渲染层使用 canvas，致使性能上会比纯 video 要求更高，但多个播放器同时存在时，若电脑性能不好或浏览器版本过低，多屏播放器就会出现卡顿问题。
   
4. 轻应用在移动端浏览器访问，ios音量打开后仍然没有声音
   
  > 受ios权限控制，需关闭手机系统静音设置
  
5. 移动端打开对讲无响应
   
  > 请先开启浏览器麦克风权限
  
6. 轻应用在移动端访问，设置了截图、屏幕录制功能配置，但是功能按钮未展示  

> IOS&Android微信小程序web-view、Android微信内置浏览器不支持截图、屏幕录制，功能按钮不展示

7. 轻应用在移动端访问，受内存限制，屏幕录制文件最大限制100M  

8. 屏幕录制下载的录制文件无法播放，如何解决？
   > 录制数据不足，建议录制时间大于5秒

9. 屏幕录制结束后，未下载 MP4 文件到本地，如何解决？
    > 可能是浏览器内存不足，文件保存失败；可重启浏览器，保留充足的内存环境

10. 移动端在退出浏览器后仍然继续播放，如何解决？
   > 可以通过监听页面可见性变化，进行暂停/恢复播放等操作，可以参考 [资源下载](/pages/a69478/) 中轻应用直播套件内demos/vue3-mobile-demo/src/views/Player.vue

   ```javascript
    // 页面处于播放状态时，监听页面可见性变化
    if (player.status.playing) {
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
   ```

11. 接入过程中出现Uncaught SyntaxError: Unexpected token '<'，WasmLib加载失败等报错信息

   > WasmLib路径引入错误，可通过参数配置WasmLibPath资源路径，如需使用绝对路径可配置："/"。(具体路径根据项目public文件路径调整)

12. 清晰度较高时，画面出现锯齿状
   > 由于画面分辨率超过客户端的渲染分辨率，canvas渲染时部分像素被丢失导致。
   > 解决方案：获取设备像素比 var pixelRatio = window.devicePixelRatio，并在初始化时配置【dpr: pixelRatio】
