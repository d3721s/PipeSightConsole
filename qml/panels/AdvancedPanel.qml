import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import PipeSightConsole

Item {
    id: panel

    Flickable {
        id: scroller
        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: scrollContent.implicitHeight
        interactive: contentHeight > height
        boundsBehavior: interactive ? Flickable.DragAndOvershootBounds : Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

        Item {
            id: scrollContent
            width: scroller.width
            height: contentLayout.implicitHeight + 32
            implicitWidth: scroller.width
            implicitHeight: contentLayout.implicitHeight + 32

            ColumnLayout {
                id: contentLayout
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 4
                spacing: 12

                GroupBox {
                    title: qsTr("导出检测报告")
                    Layout.fillWidth: true
                    RowLayout {
                        Button { text: qsTr("导出 PDF");   onClicked: pdfDlg.open() }
                        Button { text: qsTr("导出 Excel"); onClicked: xlsxDlg.open() }
                        Item { Layout.fillWidth: true }
                    }
                }

                GroupBox {
                    title: qsTr("里程计标定")
                    Layout.fillWidth: true
                    RowLayout {
                        Item { Layout.fillWidth: true }
                        Button {
                            text: qsTr("自动标定")
                            onClicked: AdvancedViewModel.calibrateOdometer()
                        }
                    }
                }

                GroupBox {
                    title: qsTr("系统日志")
                    Layout.fillWidth: true
                    RowLayout {
                        Item { Layout.fillWidth: true }
                        Button {
                            text: qsTr("打开日志")
                            onClicked: AdvancedViewModel.openSystemLog()
                        }
                    }
                }
            }
        }
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
}
