import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.OverlaySheet {
    id: rooty

    function openAndClear() {
        swipeView.currentIndex = 0
        name.text = invite.text = ""
        this.open()
    }

    QQC2.SwipeView {
        id: swipeView
        interactive: false

        RowLayout {
            anchors.centerIn: parent

            Kirigami.Card {
                implicitWidth: Kirigami.Units.gridUnit * 5
                implicitHeight: Kirigami.Units.gridUnit * 10

                ColumnLayout {
                    anchors.fill: parent

                    Kirigami.Icon {
                        source: "irc-join-channel"

                        Layout.preferredWidth: Kirigami.Units.gridUnit * 3
                        Layout.preferredHeight: Layout.preferredWidth

                        Layout.alignment: Qt.AlignHCenter
                    }

                    QQC2.Button {
                        text: "Create Guild"
                        onClicked: swipeView.currentIndex = 1

                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
            Kirigami.Card {
                implicitWidth: Kirigami.Units.gridUnit * 5
                implicitHeight: Kirigami.Units.gridUnit * 10

                ColumnLayout {
                    anchors.fill: parent

                    Kirigami.Icon {
                        source: "irc-join-channel"

                        Layout.preferredWidth: Kirigami.Units.gridUnit * 3
                        Layout.preferredHeight: Layout.preferredWidth

                        Layout.alignment: Qt.AlignHCenter
                    }

                    QQC2.Button {
                        text: "Join Guild"
                        onClicked: swipeView.currentIndex = 2

                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }
        Kirigami.FormLayout {
            QQC2.TextField {
                id: name

                Kirigami.FormData.label: "Guild Name:"
            }
            QQC2.Button {
                text: "Create Guild"

                onClicked: {
                    if (HState.createGuild(name.text)) {
                        root.showPassiveNotification("Created guild")
                    } else {
                        root.showPassiveNotification("Failed to make guild")
                    }
                    rooty.close()
                }
            }
        }
        Kirigami.FormLayout {
            QQC2.TextField {
                id: invite

                placeholderText: "harmony://..."

                Kirigami.FormData.label: "Invite Link:"
            }
            QQC2.Button {
                text: "Join"

                onClicked: {
                    if (HState.joinGuild(invite.text)) {
                        root.showPassiveNotification("Joined guild")
                    } else {
                        root.showPassiveNotification("Failed to join guild")
                    }
                    rooty.close()
                }
            }
        }
    }
}