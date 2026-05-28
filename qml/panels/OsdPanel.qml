import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PipeSightConsole

GroupBox {
    id: osdPanel
    title: qsTr("OSD 字幕叠加")
    bottomPadding: 16

    readonly property int fieldHeight: 44
    readonly property var fontOptions: ["Microsoft YaHei", "SimSun", "Arial", "Consolas"]
    readonly property var positionOptions: [qsTr("左上"), qsTr("右上"), qsTr("左下"), qsTr("右下")]
    property bool applying: false

    function setComboIndex(combo, options, value) {
        const index = options.indexOf(value)
        combo.currentIndex = index >= 0 ? index : 0
    }

    function reload() {
        showProjectInfoBox.checked = OsdViewModel.showProjectInfo
        showTimeBox.checked = OsdViewModel.showTime
        showPositionBox.checked = OsdViewModel.showPosition
        projectNameField.text = OsdViewModel.projectName
        inspectionUnitField.text = OsdViewModel.inspectionUnit
        setComboIndex(fontFamilyBox, fontOptions, OsdViewModel.fontFamily)
        fontSizeBox.value = Number(OsdViewModel.fontSize)
        positionBox.currentIndex = Math.max(0, Math.min(positionOptions.length - 1, Number(OsdViewModel.position)))
        refreshMsBox.value = Number(OsdViewModel.refreshMs)
    }

    function applyConfig() {
        applying = true
        OsdViewModel.showProjectInfo = showProjectInfoBox.checked
        OsdViewModel.showTime = showTimeBox.checked
        OsdViewModel.showPosition = showPositionBox.checked
        OsdViewModel.projectName = projectNameField.text
        OsdViewModel.inspectionUnit = inspectionUnitField.text
        OsdViewModel.fontFamily = fontFamilyBox.currentText
        OsdViewModel.fontSize = fontSizeBox.value
        OsdViewModel.position = positionBox.currentIndex
        OsdViewModel.refreshMs = refreshMsBox.value
        applying = false
        reload()
        AppNotifier.info(qsTr("OSD字幕叠加应用完成"))
    }

    Component.onCompleted: reload()

    AppConfirmDialog {
        id: confirmDialog
    }

    Connections {
        target: OsdViewModel
        function onOsdChanged() {
            if (!osdPanel.applying)
                osdPanel.reload()
        }
    }

    ColumnLayout {
        width: parent.width
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            CheckBox {
                id: showProjectInfoBox
                text: qsTr("显示项目信息")
            }
            CheckBox {
                id: showTimeBox
                text: qsTr("显示时间")
            }
            CheckBox {
                id: showPositionBox
                text: qsTr("显示当前位置(里程/深度)")
            }
            Item { Layout.fillWidth: true }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label { text: qsTr("项目名称") }
            TextField {
                id: projectNameField
                Layout.preferredWidth: 320
                Layout.preferredHeight: osdPanel.fieldHeight
            }
            Label { text: qsTr("检测单位") }
            TextField {
                id: inspectionUnitField
                Layout.preferredWidth: 320
                Layout.preferredHeight: osdPanel.fieldHeight
            }
            Item { Layout.fillWidth: true }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label { text: qsTr("字体") }
            ComboBox {
                id: fontFamilyBox
                Layout.preferredWidth: 170
                Layout.preferredHeight: osdPanel.fieldHeight
                model: osdPanel.fontOptions
            }
            Label { text: qsTr("字号") }
            SpinBox {
                id: fontSizeBox
                from: 12
                to: 96
                value: 24
                Layout.preferredWidth: 118
                Layout.preferredHeight: osdPanel.fieldHeight
            }
            Label { text: qsTr("位置") }
            ComboBox {
                id: positionBox
                Layout.preferredWidth: 96
                Layout.preferredHeight: osdPanel.fieldHeight
                model: osdPanel.positionOptions
            }
            Label { text: qsTr("刷新周期(ms)") }
            SpinBox {
                id: refreshMsBox
                from: 250
                to: 10000
                stepSize: 250
                value: 1000
                editable: true
                Layout.preferredWidth: 156
                Layout.preferredHeight: osdPanel.fieldHeight
            }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("应用")
                Layout.preferredWidth: 86
                Layout.preferredHeight: osdPanel.fieldHeight
                onClicked: confirmDialog.confirm(qsTr("确认应用OSD字幕叠加设置？"),
                                                 function() { osdPanel.applyConfig() })
            }
        }
    }
}
