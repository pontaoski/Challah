// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.14 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.OverlaySheet {
    id: rooty

	parent: applicationWindow().overlay

    function openAndClear() {
        swipeView.currentIndex = 0
        name.text = invite.text = ""
        this.open()
    }

    property Kirigami.SizeGroup sizeGroup: Kirigami.SizeGroup {
        mode: Kirigami.SizeGroup.Width
        items: [cards, makeGuild, joinGuild]
    }

    StackLayout {
        id: swipeView

        height: children[currentIndex].implicitHeight

        RowLayout {
            id: cards

            Item { implicitWidth: Kirigami.Units.largeSpacing }
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
                        text: qsTr("Create Guild")
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
                        text: qsTr("Join Guild")
                        onClicked: swipeView.currentIndex = 2

                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
            Item { implicitWidth: Kirigami.Units.largeSpacing }
        }
        Kirigami.FormLayout {
            id: makeGuild
            QQC2.TextField {
                id: name

                Kirigami.FormData.label: qsTr("Guild Name:")
            }
            QQC2.Button {
                text: qsTr("Create Guild")

                onClicked: {
                    if (HState.createGuild(name.text)) {
						//: guild has been successfully created
                        root.showPassiveNotification(qsTr("Created guild"))
                    } else {
						//: creating the guild failed
                        root.showPassiveNotification(qsTr("Failed to make guild"))
                    }
                    rooty.close()
                }
            }
        }
        Kirigami.FormLayout {
            id: joinGuild
            QQC2.TextField {
                id: invite

                placeholderText: "harmony://..."

                Kirigami.FormData.label: "Invite Link:"
            }
            QQC2.Button {
                text: qsTr("Join")

                onClicked: {
                    if (HState.joinGuild(invite.text)) {
						//: guild has been successfully joined
                        root.showPassiveNotification(qsTr("Joined guild"))
                    } else {
						//: joining the guild failed
                        root.showPassiveNotification(qsTr("Failed to join guild"))
                    }
                    rooty.close()
                }
            }
        }
    }
}
