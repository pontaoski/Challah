import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.PageRoute {
    name: "messages"
    cache: true

    Kirigami.ScrollablePage {
        Kirigami.Theme.colorSet: Kirigami.Theme.View

        footer: QQC2.ToolBar {
            RowLayout {
                anchors.fill: parent
                QQC2.TextField {
                    id: messageField
                    placeholderText: "Write a message..."

                    Layout.fillWidth: true

                    function send() {
                        Kirigami.PageRouter.data.sendMessage(text)
                        text = ""
                    }

                    onAccepted: send()
                }
                QQC2.Button {
                    text: "Send"
                    onClicked: messageField.send()
                }
            }
        }

        ListView {
            id: messagesView
            model: Kirigami.PageRouter.data
            verticalLayoutDirection: ListView.BottomToTop
            bottomMargin: 0
            topMargin: 0
            leftMargin: Kirigami.Units.gridUnit
            rightMargin: Kirigami.Units.gridUnit
            spacing: Kirigami.Units.gridUnit


            delegate: QQC2.Control {
                Kirigami.Theme.colorSet: Kirigami.Theme.Window
                padding: Kirigami.Units.gridUnit * 2

                background: Rectangle {
                    radius: 4
                    color: Kirigami.Theme.backgroundColor
                }
                contentItem: RowLayout {
                    QQC2.Label {
                        text: content

                        font.pixelSize: Kirigami.Units.gridUnit * (3/4)

                        Layout.alignment: Qt.AlignBottom
                    }
                    QQC2.Label {
                        text: date.toString()
                        opacity: 0.5

                        font.pixelSize: Kirigami.Units.gridUnit * (1/2)
                        Layout.alignment: Qt.AlignBottom
                    }
                }
            }
        }
    }
}
