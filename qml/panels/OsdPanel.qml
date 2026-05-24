import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PipeSightConsole

GroupBox {
    id: osdPanel
    title: qsTr("OSD 字幕叠加")

    readonly property int fieldHeight: 44

    ColumnLayout {
        width: parent.width
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            CheckBox {
                text: qsTr("显示项目信息")
                checked: OsdViewModel.showProjectInfo
                onToggled: OsdViewModel.showProjectInfo = checked
            }
            CheckBox {
                text: qsTr("显示时间")
                checked: OsdViewModel.showTime
                onToggled: OsdViewModel.showTime = checked
            }
            CheckBox {
                text: qsTr("显示当前位置(里程/深度)")
                checked: OsdViewModel.showPosition
                onToggled: OsdViewModel.showPosition = checked
            }
            Item { Layout.fillWidth: true }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label { text: qsTr("项目名称") }
            TextField {
                Layout.preferredWidth: 320
                Layout.preferredHeight: osdPanel.fieldHeight
                text: OsdViewModel.projectName
                onEditingFinished: OsdViewModel.projectName = text
            }
            Label { text: qsTr("检测单位") }
            TextField {
                Layout.preferredWidth: 320
                Layout.preferredHeight: osdPanel.fieldHeight
                text: OsdViewModel.inspectionUnit
                onEditingFinished: OsdViewModel.inspectionUnit = text
            }
            Item { Layout.fillWidth: true }
        }
    }
}
