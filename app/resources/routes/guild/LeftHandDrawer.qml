// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.10
import org.kde.kirigami 2.15 as Kirigami
import QtQml.Models 2.15
import com.github.HarmonyDevelopment.Challah 1.0
import Qt.labs.platform 1.1 as Labs

import "qrc:/components" as GlobalComponents

Control {
	leftPadding: 0
	rightPadding: 0
	topPadding: 0
	bottomPadding: 0

	Kirigami.Theme.inherit: false
	Kirigami.Theme.colorSet: Kirigami.Theme.View

	background: Rectangle {
		anchors.fill: parent

		Kirigami.Theme.inherit: false
		Kirigami.Theme.colorSet: Kirigami.Theme.View

		color: Kirigami.Theme.backgroundColor

		Kirigami.Separator {
			anchors {
				top: parent.top
				right: parent.right
				bottom: parent.bottom
			}
		}
	}
	contentItem: RowLayout {
		spacing: 0

		ColumnLayout {
			spacing: 0

			GlobalComponents.Header {
				z: 2
				Layout.fillWidth: true

				ToolButton {
					icon.name: "application-menu"
					onClicked: appMenu.open()

					Layout.fillWidth: true

					Menu {
						id: appMenu

						MenuItem {
							text: qsTr("Settings")
							onTriggered: settingsLoader.item.visible = true
						}

						MenuItem {
							text: qsTr("Create or Join Guild...")
							onTriggered: guildSheet.openAndClear()
						}

						MenuItem {
							text: qsTr("Log Out")
							onTriggered: CState.logOut()
						}
					}
				}
			}

			ListView {
				z: 1

				model: CState.guildList
				Layout.fillHeight: true
				Layout.fillWidth: true
				Layout.preferredWidth: 48 + (4*2)

				delegate: Control {
					id: del

					topPadding: 4
					leftPadding: 4
					rightPadding: 4
					bottomPadding: 4

					required property string guildID
					required property string guildHost

					contentItem: Kirigami.Avatar {
						implicitWidth: 48
						implicitHeight: 48

						ToolTip.text: guildData.data.name
						ToolTip.visible: maus.containsMouse

						name: guildData.data.name
						source: guildData.data.picture

						MouseArea {
							id: maus
							hoverEnabled: true
							anchors.fill: parent
							acceptedButtons: Qt.LeftButton | Qt.RightButton
							onClicked: {
								if (mouse.button == Qt.RightButton) {
									guildMenu.popup()
								} else {
									routerInstance.guildHomeserver = del.guildHost
									routerInstance.guildID = del.guildID
									routerInstance.guildIDChanged()
									otherListView.model = CState.channelsModelFor(del.guildHost, del.guildID, this)
								}
							}
						}

						Menu {
							id: guildMenu

							MenuItem {
								text: qsTr("Leave")
								onTriggered: CState.guildList.leave(del.guildHost, del.guildID)
							}
						}

						RelationalListener {
							id: guildData

							model: CState.guildsStore
							key: [del.guildHost, del.guildID]
							shape: QtObject {
								required property string name
								required property string picture
							}
						}
					}
				}
			}
		}
		Kirigami.Separator {
			visible: routerInstance.guildID != ""
			Layout.fillHeight: true
		}
		ColumnLayout {
			visible: routerInstance.guildID != ""
			spacing: 0

			GlobalComponents.Header {
				z: 2
				Layout.fillWidth: true

				RowLayout {
					Kirigami.Heading {
						level: 4
						text: tryit(() => guildData.data.name, "Guild")

						elide: Text.ElideMiddle
						Layout.fillWidth: true

						RelationalListener {
							id: guildData

							model: CState.guildsStore
							key: [routerInstance.guildHomeserver, routerInstance.guildID]
							shape: QtObject {
								required property string name
							}
						}
					}
					Button {
						icon.name: "configure"
						onClicked: if (Kirigami.Settings.isMobile) {
							colView.layers.push(Qt.resolvedUrl("qrc:/routes/guild/settings/mobile/Settings.qml"))
						} else {
							colView.layers.push(Qt.resolvedUrl("qrc:/routes/guild/settings/Settings.qml"))
						}
					}
					Layout.fillWidth: true
					Layout.margins: 8
				}
			}

			ListView {
				z: 1

				id: otherListView

				Layout.preferredWidth: 200
				Layout.fillHeight: true
				Layout.fillWidth: true

				delegate: Kirigami.BasicListItem {
					id: del

					required property string channelID

					text: channelData.data.name
					reserveSpaceForSubtitle: true

					onClicked: routerInstance.channelID = del.channelID

					leading: Item {
						width: icon.width

						Kirigami.Icon {
							id: icon

							source: {
								switch (channelData.data.kind) {
								case "voice":
									return "irc-voice"
								case "text":
									return "irc-operator"
								}
							}
							anchors.left: parent.left
							anchors.verticalCenter: parent.verticalCenter
							anchors.margins: 4
						}
					}

					RelationalListener {
						id: channelData

						model: otherListView.model.store
						key: del.channelID
						shape: QtObject {
							required property string name
							required property string kind
						}
					}
				}
			}
		}
	}
}
