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
import org.kde.kitemmodels 1.0
import com.github.HarmonyDevelopment.Challah 1.0

Kirigami.ScrollablePage {
	id: permsPage

    titleDelegate: RowLayout {
        QQC2.ToolButton {
            icon.name: "arrow-left"
            onClicked: colView.layers.pop()
        }
        Kirigami.Heading {
            text: qsTr("Permissions")
            level: 4
        }
        Item { Layout.fillWidth: true }
    }

	Kirigami.Theme.colorSet: Kirigami.Theme.View
	property var permsModel

    ColumnLayout {
		QQC2.RoundButton {
			icon.name: "list-add"
			text: qsTr("Add Permission...")
            Layout.alignment: Qt.AlignRight
			onClicked: nodeSheet.open()
		}
		QQC2.RoundButton {
			icon.name: "document-save"
			text: qsTr("Save Permissions")
			enabled: permsPage.permsModel.isDirty
            Layout.alignment: Qt.AlignRight
			onClicked: permsPage.permsModel.save()
		}

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: Kirigami.Units.largeSpacing
    }

	Kirigami.OverlaySheet {
		id: allowDenySheet

		property string node: ""

		header: Kirigami.Heading {
			text: qsTr("Allow Or Deny")
		}

		ColumnLayout {
			Kirigami.Heading {
				level: 4
				text: qsTr("Allow or deny access to this permission to members with this role?")
			}
			RowLayout {
				Item { Layout.fillWidth: true }
				QQC2.Button {
					text: qsTr("Allow")
					onClicked: {
						permsPage.permsModel.addPermission(allowDenySheet.node, true)
						allowDenySheet.close()
					}
				}
				QQC2.Button {
					text: qsTr("Deny")
					onClicked: {
						permsPage.permsModel.addPermission(allowDenySheet.node, false)
						allowDenySheet.close()
					}
				}
			}
		}
	}

	readonly property var nodeNames: {
		return {
			"actions.trigger": qsTr("Trigger Actions"),
			"channels.manage.change-name": qsTr("Change Channel Names"),
			"channels.manage.create": qsTr("Create Channels"),
			"channels.manage.delete": qsTr("Delete Channels"),
			"channels.manage.move": qsTr("Move Channels"),
			"guild.manage.change-name": qsTr("Change Guild Name"),
			"guild.manage.delete": qsTr("Delete Guild"),
			"invites.manage.create": qsTr("Create Invites"),
			"invites.view": qsTr("View Invites"),
			"messages.manage.delete": qsTr("Delete Others' Messages"),
			"messages.send": qsTr("Send Messages"),
			"messages.view": qsTr("Read Messages"),
			"permissions.manage.get": qsTr("Get Permissions"),
			"permissions.manage.set": qsTr("Set Permissions"),
			"permissions.query": qsTr("Check Permissions"),
			"roles.get": qsTr("View Roles"),
			"roles.manage": qsTr("Manage Roles"),
			"roles.users.get": qsTr("View Users' Roles"),
			"roles.users.manage": qsTr("Manage Users' Roles"),
		}
	}

	Kirigami.OverlaySheet {
		id: nodeSheet

		parent: root

		header: Kirigami.Heading {
			text: qsTr("Add Permissions")
		}
		footer: Kirigami.SearchField {
			id: searchField
			horizontalAlignment: Text.AlignLeft
		}

		ListView {
			implicitWidth: Math.min(root.width, Kirigami.Units.gridUnit * 15)
			model: KSortFilterProxyModel {
				sourceModel: ListModel {
					ListElement { section: qsTr("Actions"); node: "actions.trigger"; }
					ListElement { section: qsTr("Channels"); node: "channels.manage.change-name"; }
					ListElement { section: qsTr("Channels"); node: "channels.manage.create"; }
					ListElement { section: qsTr("Channels"); node: "channels.manage.delete"; }
					ListElement { section: qsTr("Channels"); node: "channels.manage.move"; }
					ListElement { section: qsTr("Guild"); node: "guild.manage.change-name"; }
					ListElement { section: qsTr("Guild"); node: "guild.manage.delete"; }
					ListElement { section: qsTr("Invites"); node: "invites.manage.create"; }
					ListElement { section: qsTr("Invites"); node: "invites.view"; }
					ListElement { section: qsTr("Messages"); node: "messages.manage.delete"; }
					ListElement { section: qsTr("Messages"); node: "messages.send"; }
					ListElement { section: qsTr("Messages"); node: "messages.view"; }
					ListElement { section: qsTr("Permissions"); node: "permissions.manage.get"; }
					ListElement { section: qsTr("Permissions"); node: "permissions.manage.set"; }
					ListElement { section: qsTr("Permissions"); node: "permissions.query"; }
					ListElement { section: qsTr("Roles"); node: "roles.get"; }
					ListElement { section: qsTr("Roles"); node: "roles.manage"; }
					ListElement { section: qsTr("Roles"); node: "roles.users.get"; }
					ListElement { section: qsTr("Roles"); node: "roles.users.manage"; }
				}
				filterString: searchField.text
				filterRole: "display"
				sortRole: "node"
			}
			section {
				property: "section"
				delegate: Kirigami.ListSectionHeader { label: section }
			}
			delegate: Kirigami.BasicListItem {
				text: permsPage.nodeNames[model['node']]
				topPadding: Kirigami.Units.largeSpacing
				bottomPadding: Kirigami.Units.largeSpacing

				onClicked: {
					allowDenySheet.open()
					allowDenySheet.node = model['node']
					nodeSheet.close()
				}
			}
		}
	}

	ListView {
		model: permsPage.permsModel
		delegate: Kirigami.SwipeListItem {
			id: listItem
			alwaysVisibleActions: true

			contentItem: RowLayout {
				spacing: Kirigami.Units.largeSpacing
				QQC2.Label {
					text: permsPage.nodeNames[model['nodeName']] || model['nodeName']
					verticalAlignment: Text.AlignVCenter
					Layout.fillHeight: true
				}
				Item { Layout.fillWidth: true }
				QQC2.Switch {
					checked: model['enabled']
					onCheckedChanged: {
						model['enabled'] = checked
					}
					Layout.alignment: Qt.AlignVCenter
				}
			}

			actions: [
				Kirigami.Action {
					icon.name: "edit-delete"
					onTriggered: permsPage.permsModel.deletePermission(index)
				}
			]
		}
	}
}

