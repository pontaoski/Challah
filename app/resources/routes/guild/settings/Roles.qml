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
import com.github.HarmonyDevelopment.Challah 1.0

Kirigami.ScrollablePage {
	id: rolesPage
	title: qsTr("Roles")

	Kirigami.Theme.colorSet: Kirigami.Theme.View
	property var rolesModel: CState.rolesModelFor(routerInstance.guildHomeserver, routerInstance.guildID, this)

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
				root.layers.push(Qt.resolvedUrl("Permissions.qml"), { "permsModel": rolesPage.rolesModel.everyonePermissions() })
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
					rolesPage.rolesModel.createRole(roleName.text, roleColor.color).then((ok) => {
						if (ok) {
							//: the role was created successfully
							applicationWindow().showPassiveNotification(qsTr("Created role"))
						} else {
							//: the role couldn't be created successfully
							applicationWindow().showPassiveNotification(qsTr("Failed to create role"))
						}
					})
					createRolesSheet.close()
				}
			}
		}
	}

	ListView {
		model: DelegateModel {
			id: rolesModel
			model: rolesPage.rolesModel

			delegate: Kirigami.BasicListItem {
				id: del
				required property string roleName
				required property color roleColour
				required property var permissions

				leading: Rectangle {
					width: height
					radius: height/2
					color: del.roleColour
				}

				onClicked: root.layers.push(Qt.resolvedUrl("Permissions.qml"), { "permsModel": del.permissions })

				text: roleName
			}
		}
	}
}

