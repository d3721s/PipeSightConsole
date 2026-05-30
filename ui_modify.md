**1. 状态反馈**
- 当前 PTZ 位置：Pan/Tilt `x=0.8, y=0.8`
- PTZ 移动状态：PanTilt/Zoom `IDLE`
- 当前图像参数：亮度、饱和度、对比度、锐度、IR Cut、WDR
- 当前视频编码状态：编码格式、分辨率、帧率、码率、GOP、质量
- 当前 OSD 列表和位置：`OSDTIME000/OSDTIME100`
- 当前网络接口状态：`eth0`、`wlan0`、IP、DHCP、MTU
- 当前系统时间、时区

**2. 控制命令**
- PTZ 绝对移动：`AbsoluteMove`
- PTZ 相对移动：`RelativeMove`
- PTZ 连续移动：`ContinuousMove`
- PTZ 停止：`Stop`
- PTZ 调用预置点：`GotoPreset`
- PTZ 设置/删除预置点：`SetPreset`、`RemovePreset`
- PTZ 预置巡航启动/停止：`PresetTour Start/Stop`
- 抓图：`GetSnapshotUri` 后访问图片 URL
- 拉流：`GetStreamUri` 后播放 RTSP

**3. 设定值**
- 图像亮度：`0-100`
- 图像饱和度：`0-100`
- 图像对比度：`0-100`
- 图像锐度：`0-100`
- IR Cut：`ON / OFF / AUTO`
- WDR：`ON / OFF`
- WDR Level：`0-100`
- PTZ 速度：`0.0-1.0`
- PTZ Timeout：`PT1S-PT10S`
- OSD 位置：Custom `x/y`
- OSD 时间格式、日期格式

**4. 配置参数**
- 视频编码：H.264 / H.265
- 视频分辨率：
  - `2880x1620`
  - `2560x1440`
  - `2304x1296`
  - `1920x1080`
  - `1280x960`
  - `1280x720`
- 视频帧率：`1-15`
- 视频码率：`640-4096 kbps`
- GOP：`15-150`
- 质量：`1-6`
- H.264 Profile：`Baseline / Main / High`
- H.265 Profile：`Main`
- CBR/VBR：支持 `ConstantBitRate`
- Profile 配置：视频源、视频编码、音频、PTZ
- 音频编码：G711/AAC 或 PCMU/MP4A-LATM
- 音频码率：`64 kbps`
- 音频采样率：`16 kHz`
- 组播参数：地址、端口、TTL、AutoStart
- OSD 数量限制：最多 3 个
- OSD 类型：Plain / DateAndTime
- 网络接口配置：DHCP/静态 IP、MTU
