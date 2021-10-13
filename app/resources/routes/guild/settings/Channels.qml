// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import QtQml.Models 2.15
import com.github.HarmonyDevelopment.Challah 1.0

Kirigami.ScrollablePage {
	id: pageRoot

	title: qsTr("Channels")

	actions {
		main: Kirigami.Action {
			iconName: "list-add"
			text: qsTr("Create Channel...")
			enabled: tryit(() => canCreateChannels.data.has, false)
			onTriggered: textAsker.ask(qsTr("What do you want to call the new channel?")).then((name) => delModel.model.newChannel(name))
		}
	}

	RelationalListener {
		id: canMovePositions

		model: CState.ownPermissionsStore
		key: [routerInstance.guildHomeserver, routerInstance.guildID, 0, "channels.manage.move"]
		shape: QtObject {
			required property bool has
		}
	}
	RelationalListener {
		id: canCreateChannels

		model: CState.ownPermissionsStore
		key: [routerInstance.guildHomeserver, routerInstance.guildID, 0, "channels.manage.create"]
		shape: QtObject {
			required property bool has
		}
	}

	ListView {
		id: lView

		model: DelegateModel {
			id: delModel

			model: CState.channelsModelFor(routerInstance.guildHomeserver, routerInstance.guildID, this)

			delegate: Kirigami.BasicListItem {
				id: del

				required property string channelID
				required property int index

				text: channelData.data.name
				reserveSpaceForSubtitle: true

				Drag.active: dragArea.held
				Drag.source: this
				Drag.hotSpot.x: width / 2
				Drag.hotSpot.y: height / 2

				leading: Item {
					width: icon.width
					enabled: !delModel.model.working && tryit(() => canMovePositions.data.has, false)
					Kirigami.Icon {
						id: icon
						source: "handle-sort"
						anchors {
							left: parent.left
							verticalCenter: parent.verticalCenter
						}
						MouseArea {
							id: dragArea

							anchors.fill: parent

							property bool held: false

							drag.target: held ? del : undefined
							drag.axis: Drag.YAxis

							onPressed: {
								held = true
							}
							onReleased: {
								held = false
								delModel.model.moveChannel(del.channelID, del.DelegateModel.itemsIndex)
								delModel.items.move(del.DelegateModel.itemsIndex, del.DelegateModel.itemsIndex)
							}
						}
					}
				}

				trailing: RowLayout {
					QQC2.Button {
						enabled: tryit(() => canEditChannel.data.has, false)
						text: qsTr("Rename")
						onClicked: textAsker.ask(qsTr("What do you want to rename channel to?")).then((name) => delModel.model.store.setChannelName(del.channelID, name))
					}
				}

				background: Item {
					DropArea {
						anchors.fill: parent

						onEntered: {
							delModel.items.move(
								drag.source.DelegateModel.itemsIndex,
								del.DelegateModel.itemsIndex
							)
						}
					}
				}

				RelationalListener {
					id: channelData

					model: delModel.model.store
					key: del.channelID
					shape: QtObject {
						required property string name
					}
				}
				RelationalListener {
					id: canEditChannel

					model: CState.ownPermissionsStore
					key: [routerInstance.guildHomeserver, routerInstance.guildID, del.channelID, "channels.manage.change-information"]
					shape: QtObject {
						required property bool has
					}
				}
			}
		}
		moveDisplaced: Transition {
			YAnimator {
				duration: Kirigami.Units.longDuration
				easing.type: Easing.InOutQuad
			}
		}
	}
}

