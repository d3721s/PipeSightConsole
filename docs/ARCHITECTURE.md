# PipeSightConsole 架构文档

> 管道检测机器人上位机 · Qt6/QML · 跨平台 Android + Windows

---

## 1. 目标与约束

| 维度 | 说明 |
|------|------|
| 平台 | Windows 桌面 + Android 平板/手机,同一份代码 |
| UI 框架 | Qt6 (Quick / QuickControls2) |
| 设备通信 | 主要 TCP/IP(IP 摄像头走 RTSP),少量串口可选 |
| 实时性 | 摇杆/云台等控制指令 ≤ 50ms 延迟;视频 ≤ 200ms 延迟 |
| 离线 | 录像/标记/日志需可离线工作并回放 |

---

## 2. 整体分层

```
┌───────────────────────────────────────────────────────────────┐
│                        QML View Layer                          │
│  qml/Main.qml + qml/panels/*  + qml/components/*               │
│  纯 UI · 通过 import PipeSightConsole 拿到 ViewModel 类型      │
└───────────────────────┬───────────────────────────────────────┘
                        │  Q_PROPERTY 绑定 / Q_INVOKABLE 调用
┌───────────────────────▼───────────────────────────────────────┐
│                      ViewModel Layer                           │
│  src/viewmodels/*  · Q_OBJECT + QML_ELEMENT                    │
│  每个模块一个 VM · 持有 Service 指针 · 不含业务逻辑            │
└───────────────────────┬───────────────────────────────────────┘
                        │  方法调用 / Qt 信号槽
┌───────────────────────▼───────────────────────────────────────┐
│                       Service Layer                            │
│  src/services/*  · 业务逻辑、状态机、消息编解码调度            │
│  每模块一个 · 聚合 1..N 个 TcpClient · 跨设备协作              │
└───────────────────────┬───────────────────────────────────────┘
                        │
┌───────────────────────▼───────────────────────────────────────┐
│                         Comm Layer                             │
│  src/comm/  · 运行在 worker QThread                            │
│   - TcpClient        每设备一条 TCP,自动重连+心跳             │
│   - ProtocolCodec    长度前缀帧拆包,可派生不同设备的协议       │
│   - ConnectionManager 全局命名连接池                           │
└───────────────────────┬───────────────────────────────────────┘
                        │
┌───────────────────────▼───────────────────────────────────────┐
│                          Data Layer                            │
│  src/data/  · Database(SQLite,日志/标记/报告)                │
│              · AppSettings(QSettings,IP/端口/参数)           │
└───────────────────────────────────────────────────────────────┘
```

### 分层规则(请严格遵守)

1. **QML 只跟 ViewModel 对话**,不直接访问 Service / Comm
2. **ViewModel 不实现业务**,只把 Service 状态转成 QML 友好的属性
3. **Service 通过 Comm 读写设备**,不接触 QML、不直接操作 Qt UI 线程
4. **Comm 不知道业务**,只负责字节流 ⇄ 帧
5. **Data 谁都可以读写**,但写入要走 Service(以便记录关联事件)

---

## 3. 目录结构

```
PipeSightConsole/
├── CMakeLists.txt
├── docs/
│   └── ARCHITECTURE.md          ← 本文档
├── src/
│   ├── app/
│   │   └── main.cpp             ← 启动 QML 引擎
│   ├── comm/
│   │   ├── tcp_client.{h,cpp}
│   │   ├── protocol_codec.{h,cpp}
│   │   └── connection_manager.{h,cpp}
│   ├── services/
│   │   ├── camera_service.{h,cpp}      ← 摄像头区域 + 配置/摄像头1,2
│   │   ├── laser_service.{h,cpp}       ← 激光/双目摄像头区域
│   │   ├── vehicle_service.{h,cpp}     ← 小车信息 + 控制 + 灯光
│   │   ├── osd_service.{h,cpp}         ← OSD 字幕
│   │   ├── recording_service.{h,cpp}   ← 录像配置 + 文件管理
│   │   └── report_service.{h,cpp}      ← 报告/日志/固件/AI
│   ├── viewmodels/
│   │   ├── camera_viewmodel.{h,cpp}
│   │   ├── laser_viewmodel.{h,cpp}
│   │   ├── vehicle_viewmodel.{h,cpp}
│   │   ├── osd_viewmodel.{h,cpp}
│   │   ├── config_viewmodel.{h,cpp}
│   │   └── advanced_viewmodel.{h,cpp}
│   └── data/
│       ├── database.{h,cpp}
│       └── app_settings.{h,cpp}
├── qml/
│   ├── Main.qml                       ← 应用窗口、Tab/分屏布局
│   ├── panels/
│   │   ├── CameraPanel.qml            ← 视频+云台控制
│   │   ├── LaserPanel.qml             ← 双目/点云
│   │   ├── VehicleControlPanel.qml    ← 摇杆/速度/灯光
│   │   ├── VehicleInfoPanel.qml       ← 姿态/电量/信号
│   │   ├── OsdPanel.qml               ← 字幕叠加配置
│   │   ├── ConfigPanel.qml            ← IP/码流/雷达参数
│   │   └── AdvancedPanel.qml          ← 报告/AI/日志/固件
│   └── components/
│       ├── Joystick.qml
│       ├── ConnectionIndicator.qml
│       └── VideoView.qml
└── assets/
    └── icons/
```

> **模块到表格的映射**:目录里每个 Service / Panel 直接对应需求表中的一行模块。新增一行模块就新增一对纵向切片(Service + VM + Panel)。

---

## 4. 数据流(典型场景)

### 4.1 用户按云台「右转」按钮

```
[CameraPanel.qml]                                       UI 线程
      │ onClicked: cameraVM.pan(+1, 0)
      ▼
[CameraViewModel::pan()]                                UI 线程
      │ emit cameraService->requestPan(...)             (queued)
      ▼
[CameraService::onPanRequested()]                       worker 线程
      │ codec_->encodePanCommand(...)
      │ tcpClient_->send(bytes)
      ▼
[TcpClient::send]  ─► QTcpSocket ──TCP──►  云台控制器
```

### 4.2 小车上报姿态

```
小车 ─TCP─► [TcpClient::onReadyRead]                   worker 线程
                 │  ProtocolCodec::feed(bytes) → 完整帧
                 ▼
            emit frameReceived(payload)                 (queued)
                 │
                 ▼
[VehicleService::onFrameReceived]                       worker 线程
                 │  解析为 Telemetry{pitch,roll,yaw,...}
                 │  emit telemetryUpdated(t)            (queued)
                 ▼
[VehicleViewModel] 槽函数更新 Q_PROPERTY                UI 线程
                 │
                 ▼
[VehicleInfoPanel] 通过属性绑定自动刷新                 UI 线程
```

要点:**所有跨线程通信用 Qt 信号槽 + Qt::QueuedConnection**,不要手动加锁。

---

## 5. 线程模型

| 线程 | 持有对象 | 职责 |
|------|----------|------|
| 主线程(UI) | QQmlEngine、所有 QML 对象、所有 ViewModel | 渲染、事件、动画 |
| Comm worker(1 个) | 全部 TcpClient、ConnectionManager | 所有 TCP 读写 |
| 媒体线程(Qt Multimedia 内部) | VideoSink 解码 | RTSP 解码、视频帧 |
| 任务线程(QtConcurrent / QThreadPool) | 一次性重活 | 报告导出、AI 推理、文件 IO |

**Service 跑在哪?** 推荐**跟它的主要 TcpClient 同线程**(即 Comm worker)。这样 Service 直接调用 TcpClient 的方法不用排队,只在跟 ViewModel 通信时跨线程。

如果某 Service 完全无 IO(比如 OsdService 只是状态聚合),放主线程也行。

---

## 6. 通信协议规范

### 6.1 帧格式(推荐二进制)

```
┌────────┬────────┬──────────┬────────────────┬────────┐
│ MAGIC  │  LEN   │   TYPE   │     PAYLOAD     │  CRC   │
│ 2 byte │ 2 byte │  1 byte  │   LEN-4 byte    │ 2 byte │
└────────┴────────┴──────────┴────────────────┴────────┘
  0xA55A   uint16    uint8       device-specific  CRC16
```

- `MAGIC`:固定 `0xA55A`,用于流中重同步
- `LEN`:从 TYPE 到 CRC 的字节数
- `TYPE`:消息类型(0x01 心跳、0x10 控制、0x20 遥测、0x30 配置...)
- `CRC`:CRC16-CCITT,覆盖 TYPE+PAYLOAD

`ProtocolCodec` 实现 `feed(QByteArray) → QList<Frame>`,处理粘包/拆包/重同步。

> **若硬件已有协议**:直接派生 `ProtocolCodec` 实现 `encode/decode`,业务层不变。

### 6.2 IP 摄像头(独立通道)

不走 `TcpClient`,走 Qt Multimedia 的 `QMediaPlayer + RTSP URL`(`rtsp://user:pwd@ip:554/stream1`)。`CameraService` 持有 `QMediaPlayer`,把 `videoSink()` 暴露给 QML 的 `VideoView`。

---

## 7. 跨平台注意点

### Windows
- 编译器:MSVC 2022 或 MinGW(Qt 自带)
- 视频后端:DirectShow / WMF(Qt 默认)
- 部署:`windeployqt` 拷依赖

### Android
- 最低 API 24(Android 7.0)
- 权限:`INTERNET`、`ACCESS_NETWORK_STATE`、`WRITE_EXTERNAL_STORAGE`(报告/录像导出)
- 视频后端:GStreamer 或 MediaCodec(Qt 默认走系统)
- UI 适配:`Material` style;摇杆/按钮按触摸尺寸(≥ 48dp)
- 横屏锁定:`AndroidManifest.xml` 设 `screenOrientation="landscape"`

> Android 包源目录:`android/`(首次在 Qt Creator 选 Android Kit 构建时自动生成)

---

## 8. 如何加一个新模块(纵向切片)

假设要加「气体传感器」模块:

1. **Comm**:决定共用现有 `TcpClient` 还是新加一条;若协议不同,派生 `ProtocolCodec`
2. **Service**:`src/services/gas_service.{h,cpp}`,接收帧 → 解析 → 发出 `gasReadingUpdated(ppm)` 信号
3. **ViewModel**:`src/viewmodels/gas_viewmodel.{h,cpp}`,`Q_PROPERTY(double ppm READ ppm NOTIFY ppmChanged)`
4. **QML**:`qml/panels/GasPanel.qml`,绑定 `gasVM.ppm`,放进 `Main.qml` 的 Tab/分屏
5. **CMakeLists.txt**:把新文件加进对应 `*_SOURCES` 列表

不需要改任何其他模块。

---

## 9. 关键依赖与版本

| 库 | 版本 | 用途 |
|----|------|------|
| Qt | 6.5+ | UI / 网络 / 多媒体 / SQLite |
| C++ | 17 | 标准 |
| CMake | 3.21+ | 构建 |
| (可选) FFmpeg | 5.x | 视频回退方案 |
| (可选) ONNX Runtime | 1.16+ | AI 缺陷识别 |
| (可选) QXlsx | latest | Excel 报告导出 |

---

## 10. 构建

### Windows
```powershell
# 在 Qt Creator 中打开 CMakeLists.txt,选 MSVC 2022 Kit,Build
# 或命令行:
cmake -B build -G "Ninja"
cmake --build build
```

### Android
```
Qt Creator → Open → CMakeLists.txt → 选 Android arm64-v8a Kit
首次 Build 会生成 android/ 目录,在里面配置 AndroidManifest.xml
```

---

## 11. 已知决策(写下来避免反复)

| 决策 | 取舍 | 当前选择 | 何时回头 |
|------|------|----------|----------|
| 视频后端 | Qt Multimedia vs FFmpeg | **Qt Multimedia** | 若 Android RTSP 兼容性差则切 FFmpeg |
| 协议格式 | JSON vs 二进制 | **二进制+长度前缀** | 硬件如果已有 JSON 协议则跟随硬件 |
| Service 线程 | UI 线程 vs Worker | **跟 TcpClient 同 worker** | 几乎不需要回头 |
| 配置存储 | QSettings vs SQLite | **QSettings** 存简单 K-V,**SQLite** 存结构化(标记/日志) | |
| UI Style | Basic vs Fusion vs Material | **Material** (Android 自然,Windows 也好看) | 若用户要更"工业风"换 Universal/Fusion |

---

## 12. 待办与扩展点

- [ ] 实际硬件协议确认后,完成 `ProtocolCodec` 派生类
- [ ] AI 缺陷识别(`AdvancedFeatures::recognizeDefects`)接 ONNX Runtime
- [ ] 报告导出接 QtPrintSupport(PDF)+ QXlsx(Excel)
- [ ] Android 端横屏锁定 + 权限申请代码
- [ ] 单元测试目录 `tests/`(Qt Test)
