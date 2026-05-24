import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import PipeSightConsole

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        GroupBox {
            title: qsTr("导出检测报告")
            Layout.fillWidth: true
            RowLayout {
                Button { text: qsTr("导出 PDF");   onClicked: pdfDlg.open() }
                Button { text: qsTr("导出 Excel"); onClicked: xlsxDlg.open() }
                ProgressBar {
                    from: 0; to: 100; value: AdvancedViewModel.reportProgress
                    Layout.fillWidth: true
                }
            }
        }

        GroupBox {
            title: qsTr("里程计标定")
            Layout.fillWidth: true
            RowLayout {
                Label { text: qsTr("已知行进距离 (m)") }
                TextField { id: knownDist; Layout.preferredWidth: 120; text: "10.0" }
                Button {
                    text: qsTr("开始标定")
                    onClicked: AdvancedViewModel.calibrateOdometer(parseFloat(knownDist.text))
                }
            }
        }

        GroupBox {
            title: qsTr("AI 缺陷识别")
            Layout.fillWidth: true
            RowLayout {
                TextField { id: aiPath; Layout.fillWidth: true; placeholderText: qsTr("视频文件或文件夹") }
                Button { text: qsTr("浏览...");  onClicked: aiFileDlg.open() }
                Button {
                    text: qsTr("开始识别")
                    onClicked: AdvancedViewModel.runDefectRecognition(aiPath.text)
                }
            }
        }

        GroupBox {
            title: qsTr("回放与标记编辑")
            Layout.fillWidth: true
            RowLayout {
                TextField { id: pbPath; Layout.fillWidth: true; placeholderText: qsTr("会话目录") }
                Button { text: qsTr("打开"); onClicked: AdvancedViewModel.openPlayback(pbPath.text) }
            }
        }

        GroupBox {
            title: qsTr("系统日志")
            Layout.fillWidth: true
            RowLayout {
                Label { text: AdvancedViewModel.systemLogPath; Layout.fillWidth: true; elide: Label.ElideMiddle }
                Button { text: qsTr("打开日志位置") /* TODO: open in shell */ }
            }
        }

        GroupBox {
            title: qsTr("小车固件升级")
            Layout.fillWidth: true
            ColumnLayout {
                RowLayout {
                    TextField { id: carFwPath; Layout.fillWidth: true; placeholderText: qsTr("小车固件文件路径") }
                    Button { text: qsTr("浏览..."); onClicked: carFwFileDlg.open() }
                    Button {
                        text: qsTr("开始升级")
                        onClicked: AdvancedViewModel.uploadFirmware("car", carFwPath.text)
                    }
                }
                ProgressBar {
                    from: 0; to: 100
                    value: AdvancedViewModel.firmwareProgress
                    Layout.fillWidth: true
                }
            }
        }

        GroupBox {
            title: qsTr("相机固件升级")
            Layout.fillWidth: true
            ColumnLayout {
                RowLayout {
                    Label { text: qsTr("目标相机") }
                    ComboBox {
                        id: cameraFwTarget
                        model: [qsTr("前摄"), qsTr("后摄")]
                        Layout.preferredWidth: 160
                    }
                    TextField { id: cameraFwPath; Layout.fillWidth: true; placeholderText: qsTr("相机固件文件路径") }
                    Button { text: qsTr("浏览..."); onClicked: cameraFwFileDlg.open() }
                    Button {
                        text: qsTr("开始升级")
                        onClicked: AdvancedViewModel.uploadFirmware(
                                       cameraFwTarget.currentIndex === 0 ? "camera-front" : "camera-rear",
                                       cameraFwPath.text)
                    }
                }
                ProgressBar {
                    from: 0; to: 100
                    value: AdvancedViewModel.firmwareProgress
                    Layout.fillWidth: true
                }
            }
        }

        Item { Layout.fillHeight: true }
    }

    FileDialog {
        id: pdfDlg
        nameFilters: ["PDF (*.pdf)"]
        fileMode: FileDialog.SaveFile
        onAccepted: AdvancedViewModel.exportPdf(selectedFile)
    }
    FileDialog {
        id: xlsxDlg
        nameFilters: ["Excel (*.xlsx)"]
        fileMode: FileDialog.SaveFile
        onAccepted: AdvancedViewModel.exportExcel(selectedFile)
    }
    FileDialog {
        id: aiFileDlg
        nameFilters: ["Videos (*.mp4 *.mkv *.avi)", "All files (*)"]
        onAccepted: aiPath.text = selectedFile
    }
    FileDialog {
        id: carFwFileDlg
        nameFilters: ["Firmware (*.bin *.fw *.zip)", "All files (*)"]
        onAccepted: carFwPath.text = selectedFile
    }
    FileDialog {
        id: cameraFwFileDlg
        nameFilters: ["Firmware (*.bin *.fw *.zip)", "All files (*)"]
        onAccepted: cameraFwPath.text = selectedFile
    }
}
