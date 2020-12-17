// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import QtQuick.Dialogs 1.3
import QtQml.Models 2.15
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.ScrollablePage {
	title: qsTr("Roles")

	Kirigami.Theme.colorSet: Kirigami.Theme.View

	actions {
		main: Kirigami.Action {
			iconName: "list-add"
			text: qsTr("Create Role...")
			onTriggered: {
				roleName.text = ""
				createRolesSheet.open()
			}
		}
		right: Kirigami.Action {
			iconName: "document-edit"
			text: qsTr("Edit Default Permissions...")
			onTriggered: {
				root.layers.push(Qt.resolvedUrl("Permissions.qml"), { "permsModel": settingsPage.rolesModel.everyonePermissions() })
			}
		}
	}

	ColorDialog {
		id: colorDialog
		onAccepted: roleColor.color = color
	}

	Kirigami.OverlaySheet {
		id: createRolesSheet

		parent: applicationWindow().overlay

		Kirigami.FormLayout {
			QQC2.TextField {
				id: roleName
				Kirigami.FormData.label: qsTr("Role Name:")
			}
			QQC2.Button {
				id: roleColor
				property color color: "red"
				onClicked: colorDialog.open()

				contentItem: Rectangle {
					color: roleColor.color
					implicitHeight: Kirigami.Units.gridUnit * 2
					implicitWidth: Kirigami.Units.gridUnit * 2
				}
				Kirigami.FormData.label: qsTr("Role Color:")
			}
			QQC2.Button {
				text: qsTr("Create Role")

				onClicked: {
					if (settingsPage.rolesModel.createRole(roleName.text, roleColor.color)) {
						//: the role was created successfully
						applicationWindow().showPassiveNotification(qsTr("Created role"))
					} else {
						//: the role couldn't be created successfully
						applicationWindow().showPassiveNotification(qsTr("Failed to create role"))
					}
					createRolesSheet.close()
				}
			}
		}
	}

	ListView {
		model: DelegateModel {
			id: rolesModel
			model: settingsPage.rolesModel

			delegate: Item {
				id: delegateRoot

				implicitHeight: listItem.implicitHeight
				implicitWidth: listItem.implicitWidth
				anchors {
					left: parent.left
					right: parent.right
				}

				Drag.active: dragArea.held
				Drag.source: this
				Drag.hotSpot.x: width / 2
				Drag.hotSpot.y: height / 2

				Kirigami.SwipeListItem {
					id: listItem
					anchors.fill: parent

					contentItem: RowLayout {
						spacing: Kirigami.Units.largeSpacing
						Rectangle {
							implicitHeight: Kirigami.Units.gridUnit
							implicitWidth: Kirigami.Units.gridUnit
							radius: Kirigami.Units.gridUnit
							color: model['roleColour']
							Layout.alignment: Qt.AlignVCenter
						}
						QQC2.Label {
							text: model['roleName']
							verticalAlignment: Text.AlignVCenter
							Layout.fillHeight: true
						}
						Item { Layout.fillWidth: true }
					}

					actions: [
						Kirigami.Action {
							icon.name: "edit-delete"
							onTriggered: print("not implemented")
						}
					]
				}

				MouseArea {
					id: dragArea

					property bool held: false
					anchors.fill: parent

					drag.target: held ? delegateRoot : undefined
					drag.axis: Drag.YAxis

					onClicked: {
						root.layers.push(Qt.resolvedUrl("Permissions.qml"), { "permsModel": model['permissions'] })
					}
					onPressAndHold: {
						delegateRoot.z = 0
						held = true
					}
					onReleased: {
						delegateRoot.z = 999
						held = false
						rolesModel.model.moveRoleFromTo(index, delegateRoot.DelegateModel.itemsIndex)
						rolesModel.items.move(delegateRoot.DelegateModel.itemsIndex, delegateRoot.DelegateModel.itemsIndex)
					}

					DropArea {
						anchors.fill: parent

						onEntered: {
							rolesModel.items.move(
								drag.source.DelegateModel.itemsIndex,
								delegateRoot.DelegateModel.itemsIndex
							)
						}
					}
				}
			}
		}
	}
}

