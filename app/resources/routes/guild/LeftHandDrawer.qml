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
				Layout.preferredWidth: 48

				delegate: Kirigami.Avatar {
					id: del

					implicitWidth: 48
					implicitHeight: 48

					required property string guildID
					required property string guildHost

					ToolTip.text: guildData.data.name
					ToolTip.visible: maus.containsMouse

					name: guildData.data.name
					source: guildData.data.picture

					MouseArea {
						id: maus
						hoverEnabled: true
						anchors.fill: parent
						onClicked: {
							routerInstance.guildHomeserver = del.guildHost
							routerInstance.guildID = del.guildID
							routerInstance.guildIDChanged()
							otherListView.model = CState.channelsModelFor(del.guildHost, del.guildID, this)
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

				Kirigami.Heading {
					level: 4
					text: tryit(() => guildData.data.name, "Guild")

					RelationalListener {
						id: guildData

						model: CState.guildsStore
						key: [routerInstance.guildHomeserver, routerInstance.guildID]
						shape: QtObject {
							required property string name
						}
					}
				}
			}

			ListView {
				z: 1

				id: otherListView

				Layout.preferredWidth: 200
				Layout.fillHeight: true

				delegate: Kirigami.BasicListItem {
					id: del

					required property string channelID

					text: channelData.data.name
					reserveSpaceForSubtitle: true

					onClicked: routerInstance.channelID = del.channelID

					RelationalListener {
						id: channelData

						model: otherListView.model.store
						key: del.channelID
						shape: QtObject {
							required property string name
						}
					}
				}
			}
			ToolBar {
				z: 2

				position: ToolBar.Footer

				Layout.fillWidth: true

				contentItem: RowLayout {
					Button {
						text: qsTr("Guild Configuration")
						onClicked: root.pageStack.layers.push(Qt.resolvedUrl("qrc:/routes/guild/settings/Settings.qml"))
					}
					Item { Layout.fillWidth: true }
					Button {
						icon.name: "list-add"
						onClicked: channelMenu.open()
					}

					Labs.Menu {
						id: channelMenu

						Labs.MenuItem {
							text: qsTr("New Channel")
							onTriggered: textAsker.ask(qsTr("What do you want to call the new channel?")).then((name) => otherListView.model.newChannel(name))
						}
						Labs.MenuItem {
							text: qsTr("New Category")
							enabled: false
						}
					}
				}
			}
		}
	}
}
