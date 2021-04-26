// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.10
import org.kde.kirigami 2.15 as Kirigami
import QtQml.Models 2.15
import com.github.HarmonyDevelopment.Challah 1.0

Control {
	leftPadding: 0
	rightPadding: 0
	topPadding: 0
	bottomPadding: 0
	background: Rectangle {
		anchors.fill: parent

		Kirigami.Theme.inherit: true
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

		Kirigami.SizeGroup {
			mode: Kirigami.SizeGroup.Height
			items: [lhToolbar, rhToolbar]
		}

		ColumnLayout {
			spacing: 0

			ToolBar {
				id: lhToolbar
				z: 2

				Layout.fillWidth: true

				contentItem: ToolButton {
					icon.name: "application-menu"
					onClicked: appMenu.open()

					Layout.fillWidth: true

					Menu {
						id: appMenu

						MenuItem {
							text: qsTr("Settings")
							onTriggered: root.pageStack.layers.push(Qt.resolvedUrl("ChallahSettings.qml"), {})
						}
						MenuItem {
							text: qsTr("Log Out")
							onTriggered: HState.logOut()
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

					MouseArea {
						id: maus
						hoverEnabled: true
						anchors.fill: parent
						onClicked: {
							routerInstance.guildHomeserver = del.guildHost
							routerInstance.guildID = del.guildID
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

			ToolBar {
				id: rhToolbar
				z: 2
				Layout.fillWidth: true
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

					onClicked: {
						routerInstance.channelID = del.channelID
						routerInstance.navigateToRoute(["Guild/Blank", {
							"route": "Guild/Timeline",
							"homeserver": routerInstance.guildHomeserver,
							"guildID": routerInstance.guildID,
							"channelID": del.channelID,
						}])
					}

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
		}
	}
}
