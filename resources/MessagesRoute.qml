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
        id: messagesRoute
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        property var model: Kirigami.PageRouter.data

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

            section {
                criteria: ViewSection.FullString
                property: "authorName"
                delegate: RowLayout {
                    required property string section
                    required property string authorAvatar

                    Kirigami.Avatar {
                        name: parent.section
                        source: parent.authorAvatar

                        Layout.preferredHeight: Kirigami.Units.gridUnit * 1.5
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 1.5
                    }
                    Kirigami.Heading {
                        level: 4
                        text: parent.section

                        Layout.fillWidth: true
                    }
                }
            }

            delegate: ColumnLayout {
                id: messageDelegate
                property string modelMessageID: messageID

                QQC2.Control {
                    id: messageBlock
                    property bool edit: false

                    padding: Kirigami.Units.gridUnit * 2

                    Layout.minimumWidth: Kirigami.Units.gridUnit * 3
                    Layout.maximumWidth: applicationWindow().wideScreen ? Math.max(messagesView.width / 3, Kirigami.Units.gridUnit * 15) : (messagesView.width * 0.9)
                    Kirigami.Theme.colorSet: Kirigami.Theme.Window

                    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth + leftPadding + rightPadding)
                    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, implicitContentHeight + topPadding + bottomPadding)

                    background: Rectangle {
                        radius: 4
                        color: Kirigami.Theme.backgroundColor

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.RightButton
                            onClicked: {
                                if ((messagesRoute.model.userID() == authorID || messagesRoute.model.isOwner()) && mouse.button === Qt.RightButton)
                                    messageMenu.popup()
                            }
                        }
                        QQC2.Menu {
                            id: messageMenu

                            QQC2.MenuItem {
                                text: "Edit"
                                visible: messagesRoute.model.userID() == authorID
                                onTriggered: {
                                    messageBlock.edit = true
                                }
                            }
                            QQC2.MenuItem {
                                text: "Delete"
                                visible: messagesRoute.model.userID() == authorID || messagesRoute.model.isOwner()
                                onTriggered: {
                                    messagesRoute.model.deleteMessage(messageID)
                                }
                            }
                        }
                    }
                    contentItem: ColumnLayout {
                        QQC2.Label {
                            visible: !messageBlock.edit
                            text: content

                            font.pixelSize: Kirigami.Units.gridUnit * (3/4)
                            wrapMode: Text.Wrap

                            Layout.alignment: Qt.AlignBottom
                            Layout.maximumWidth: messageBlock.Layout.maximumWidth * 0.9
                        }
                        QQC2.TextField {
                            visible: messageBlock.edit
                            text: content

                            Keys.onEscapePressed: messageBlock.edit = false

                            onAccepted: {
                                messageBlock.edit = false
                                messagesRoute.model.editMessage(messageID, text)
                            }

                            Layout.alignment: Qt.AlignBottom
                        }
                        QQC2.Label {
                            text: `${("0"+date.getHours()).slice(-2)}:${("0"+date.getMinutes()).slice(-2)}`
                            opacity: 0.5

                            font.pixelSize: Kirigami.Units.gridUnit * (1/2)
                            Layout.alignment: Qt.AlignBottom | Qt.AlignRight
                        }
                    }
                }
                Repeater {
                    model: actions
                    delegate: MessageAction {
                        messageID: messageDelegate.modelMessageID
                    }
                }
                Repeater {
                    model: embeds
                    delegate: Embed {}
                }
            }

            add: Transition {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration
                    properties: "y"
                    easing.type: Easing.InOutCubic
                }
            }
            displaced: Transition {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration
                    properties: "y"
                    easing.type: Easing.InOutCubic
                }
            }
        }
    }
}
