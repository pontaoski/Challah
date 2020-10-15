// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.10
import org.kde.kirigami 2.11 as Kirigami
import com.github.HarmonyDevelopment.Staccato 1.0

Item {
	id: drawer

	implicitWidth: shouldShow ? (channelsView.model != null ? 72 + 200 : 72) : 0
	property bool shouldShow: false

	RowLayout {
		spacing: 0
		anchors.fill: parent

		ScrollView {
			id: scrollView

			Kirigami.Theme.inherit: true
			anchors.fill: parent
			ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
			ScrollBar.vertical.anchors {
				top: scrollView.top
				bottom: scrollView.bottom
			}

			Flickable {
				contentWidth: 72

				ColumnLayout {
					width: 72
					implicitWidth: 72

					Rectangle {
						implicitWidth: 48
						implicitHeight: 48
						color: Kirigami.Theme.backgroundColor
						radius: 48 / 2

						ToolButton {
							anchors.fill: parent
							icon.name: "list-add"
							onClicked: guildSheet.openAndClear()
						}

						Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
					}

					Repeater {
						model: HState.guildModel
						delegate: Rectangle {
							implicitWidth: 48
							implicitHeight: 48
							color: "cyan"
							radius: 48 / 2

							Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

							ToolTip.text: model['guildName']
							ToolTip.visible: maus.containsMouse

							MouseArea {
								id: maus
								anchors.fill: parent
								hoverEnabled: true
								acceptedButtons: Qt.LeftButton | Qt.RightButton
								onClicked: {
									if (mouse.button === Qt.RightButton) {
										guildMenu.popup()
										return
									}

									channelsTitle.text = model['guildName']
									channelsView.model = model['channelModel']

									rightHandDrawer.model = model['channelModel'].members
								}
							}

							Menu {
								id: guildMenu

								MenuItem {
									text: model['isOwner'] ? "Delete" : "Leave"
									onTriggered: {
										if (HState.leaveGuild(model['homeserver'], model['guildID'], model['isOwner'])) {
											root.showPassiveNotification("Left guild")
										} else {
											root.showPassiveNotification("Failed to leave guild")
										}
									}
								}
							}
						}
					}
				}
			}
		}
		Kirigami.Separator {
			Layout.fillHeight: true

			visible: channelsView.model != null
		}
		ColumnLayout {
			spacing: 0
			visible: channelsView.model != null

			Kirigami.ApplicationHeader {
				z: 2
				Layout.fillWidth: true

				contentItem: RowLayout {
					anchors {
						verticalCenter: parent.verticalCenter
						left: parent.left
						right: parent.right
					}

					Kirigami.Heading {
						id: channelsTitle
						leftPadding: Kirigami.Units.largeSpacing
						text: "Channels"
						Layout.fillWidth: true
					}
					ToolButton {
						icon.name: "settings-configure"
						onClicked: root.pageStack.layers.push(Qt.resolvedUrl("Invites.qml"), {"inviteModel": channelsView.model.invitesModel()})
					}
					ToolButton {
						icon.name: "list-add"
						onClicked: sheety.open()
					}
				}
				pageDelegate: Item {}
			}
			ListView {
				id: channelsView

				z: 1

				Layout.preferredWidth: 198
				Layout.fillHeight: true

				delegate: Kirigami.SwipeListItem {
					contentItem: Label {
						text: `#${channelName}`
						anchors.verticalCenter: parent.verticalCenter
						verticalAlignment: Text.AlignVCenter
					}

					onClicked: {
						routerInstance.navigateToRoute(
							{
								"route": "messages",
								"data": messagesModel,
								"title": `#${channelName}`
							}
						)
					}

					actions: [
						Kirigami.Action {
							icon.name: "edit-delete"
							onTriggered: channelsView.model.deleteChannel(channelID)
						}
					]
				}
			}
		}
	}

	ChannelSheet {
		id: sheety
		model: channelsView.model
	}
}
