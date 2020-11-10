// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQml.Models 2.15
import com.github.HarmonyDevelopment.Staccato 1.0

Item {
	id: drawer

	implicitWidth: shouldShow ? (channelsModel.model != null ? 72 + 200 : 72) : 0
	property bool shouldShow: false

	Rectangle {
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
							icon.name: "application-menu"
							onClicked: appMenu.open()
						}

						Menu {
							id: appMenu

							MenuItem {
								text: "Log Out"
								onTriggered: routerInstance.navigateToRoute("login")
							}
						}

						Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
					}

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
						delegate: Avatar {
							implicitWidth: 48
							implicitHeight: 48

							name: model['guildName']
							source: model['picture']

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
									channelsModel.model = model['channelModel']

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

			visible: channelsModel.model != null
		}
		ColumnLayout {
			spacing: 0
			visible: channelsModel.model != null

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
						onClicked: colView.layers.push(Qt.resolvedUrl("Invites.qml"), {"inviteModel": channelsModel.model.invitesModel()})
					}
					ToolButton {
						icon.name: "list-add"
						onClicked: sheety.open()
					}
				}
				pageDelegate: Item {}
			}
			ListView {
				z: 1

				Kirigami.Theme.inherit: true
				Kirigami.Theme.colorSet: Kirigami.Theme.View

				Layout.preferredWidth: 198
				Layout.fillHeight: true

				moveDisplaced: Transition {
					YAnimator {
						duration: Kirigami.Units.longDuration
						easing.type: Easing.InOutQuad
					}
				}

				model: DelegateModel {
					id: channelsModel

					delegate: Item {
						id: itemer
						implicitWidth: swipeyDelegate.implicitWidth
						implicitHeight: swipeyDelegate.implicitHeight
						anchors {
							left: parent.left
							right: parent.right
						}

						Drag.active: dragArea.held
						Drag.source: this
						Drag.hotSpot.x: width / 2
						Drag.hotSpot.y: height / 2

						Kirigami.SwipeListItem {
							id: swipeyDelegate
							anchors.fill: parent

							contentItem: RowLayout {
								Label {
									text: `#${channelName}`
									anchors.verticalCenter: parent.verticalCenter
									verticalAlignment: Text.AlignVCenter
								}
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
									onTriggered: channelsModel.model.deleteChannel(channelID)
								}
							]
						}

						MouseArea {
							id: dragArea

							property bool held: false
							anchors.fill: swipeyDelegate

							drag.target: held ? itemer : undefined
							drag.axis: Drag.YAxis

							onClicked: {
								routerInstance.navigateToRoute(
									{
										"route": "messages",
										"data": messagesModel,
										"title": `#${channelName}`
									}
								)
							}
							onPressAndHold: {
								itemer.z = 0
								held = true
							}
							onReleased: {
								itemer.z = 999
								held = false
								channelsModel.model.moveChannelFromTo(index, itemer.DelegateModel.itemsIndex)
								channelsModel.items.move(itemer.DelegateModel.itemsIndex, itemer.DelegateModel.itemsIndex)
							}

							DropArea {
								anchors.fill: parent

								onEntered: {
									channelsModel.items.move(
										drag.source.DelegateModel.itemsIndex,
										itemer.DelegateModel.itemsIndex
									)
								}
							}
						}
					}
				}
			}
		}
	}

	ChannelSheet {
		id: sheety
		model: channelsModel.model
	}
}
