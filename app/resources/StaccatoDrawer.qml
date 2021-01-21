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

	implicitWidth: shouldShow ? (channelsModel.workaroundModel != null ? 72 + 200 : 72) : 0
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
			Layout.preferredWidth: 72
			Layout.fillHeight: true
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

					Kirigami.ApplicationHeader {
						Layout.fillWidth: true

						contentItem: ToolButton {
							anchors.fill: parent
							icon.name: "application-menu"
							onClicked: appMenu.open()

							Layout.fillWidth: true

							ResponsiveMenu {
								id: appMenu

								ResponsiveMenuItem {
									text: qsTr("Settings")
									onTriggered: root.pageStack.layers.push(Qt.resolvedUrl("ChallahSettings.qml"), {})
								}
								ResponsiveMenuItem {
									text: qsTr("Log Out")
									onTriggered: HState.logOut()
								}
							}
						}
						pageDelegate: Item {}

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
						delegate: Kirigami.Avatar {
							implicitWidth: 48
							implicitHeight: 48

							name: guildName
							source: picture

							Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

							ToolTip.text: guildName
							ToolTip.visible: maus.containsMouse

							MouseArea {
								id: maus
								anchors.fill: parent
								hoverEnabled: true
								acceptedButtons: Qt.LeftButton | Qt.RightButton
								onClicked: {
									if (mouse.button === Qt.RightButton) {
										guildMenu.open()
										return
									}

									routerInstance.navigateToRoute({
										"route": "guild",
										"guildID": model['guildID'],
										"homeserver": model['homeserver'],
									})
								}
							}

							ResponsiveMenu {
								id: guildMenu

								ResponsiveMenuItem {
									text: model['isOwner'] ? qsTr("Delete") : qsTr("Leave")
									onTriggered: {
										if (HState.leaveGuild(model['homeserver'], model['guildID'], model['isOwner'])) {
											//: guild leaving succeeded
											root.showPassiveNotification(qsTr("Left guild"))
										} else {
											//: guild leaving failed
											root.showPassiveNotification(qsTr("Failed to leave guild"))
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

			visible: channelsModel.workaroundModel != null
		}
		ColumnLayout {
			spacing: 0
			visible: channelsModel.workaroundModel != null
			Layout.fillWidth: true

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
						leftPadding: Kirigami.Units.largeSpacing
						text: qsTr("Channels")
						Layout.fillWidth: true
					}
					ToolButton {
						icon.name: "settings-configure"
						visible: channelsModel.workaroundModel.permissions.canViewInvites || channelsModel.workaroundModel.permissions.canManageRoles
						onClicked: root.pageStack.layers.push(Qt.resolvedUrl("GuildSettings.qml"), {
							"invitesModel": channelsModel.workaroundModel.permissions.canViewInvites ? channelsModel.workaroundModel.invitesModel() : null,
							"rolesModel": channelsModel.workaroundModel.permissions.canManageRoles ? channelsModel.workaroundModel.rolesModel() : null
						})
					}
					ToolButton {
						icon.name: "list-add"
						visible: channelsModel.workaroundModel.permissions.canCreate
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
				Layout.fillWidth: true
				Layout.rightMargin: 1
				Layout.fillHeight: true

				moveDisplaced: Transition {
					YAnimator {
						duration: Kirigami.Units.longDuration
						easing.type: Easing.InOutQuad
					}
				}

				model: DelegateModel {
					id: channelsModel

					model: workaroundModel
					property var workaroundModel: HState.channelsModel(routerInstance.params.guildID, routerInstance.params.homeserver)

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
									verticalAlignment: Text.AlignVCenter
								}
							}

							actions: [
								Kirigami.Action {
									icon.name: "edit-delete"
									onTriggered: channelsModel.workaroundModel.deleteChannel(channelID)
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
								if (!!routerInstance.params.channelID) {
									routerInstance.popRoute()
								}
								routerInstance.pushRoute(
									{
										"route": "messages",
										"title": `#${channelName}`,
										"channelID": channelID,
									}
								)
							}
							onPressAndHold: {
								if (channelsModel.workaroundModel.permissions.canMove) {
									itemer.z = 0
									held = true
								}
							}
							onReleased: {
								if (held) {
									itemer.z = 999
									held = false
									channelsModel.workaroundModel.moveChannelFromTo(index, itemer.DelegateModel.itemsIndex)
									channelsModel.items.move(itemer.DelegateModel.itemsIndex, itemer.DelegateModel.itemsIndex)
								}
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
		model: channelsModel.workaroundModel
	}
}
