// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.ScrollablePage {
	id: invitePage
	title: qsTr("Invites")

	property var inviteModel

	actions {
		main: Kirigami.Action {
			iconName: "list-add"
			text: qsTr("Create Invite...")
			onTriggered: {
				inviteField.text = ""
				inviteCount.value = 0
				inviteCount.Kirigami.FormData.checked = false
				createInvitesSheet.open()
			}
		}
	}

	Kirigami.Theme.colorSet: Kirigami.Theme.View

	Kirigami.OverlaySheet {
		id: createInvitesSheet

		parent: applicationWindow().overlay

		Kirigami.FormLayout {
			QQC2.TextField {
				id: inviteField

				Kirigami.FormData.label: qsTr("Invite ID:")
			}
			QQC2.SpinBox {
				id: inviteCount
				enabled: Kirigami.FormData.checked

				from: 1
				to: 1000000
				stepSize: 1

				Kirigami.FormData.label: qsTr("Restrict uses:")
				Kirigami.FormData.checkable: true
			}
			QQC2.Button {
				text: qsTr("Create Invite")

				onClicked: {
					if (invitePage.inviteModel.createInvite(inviteField.text, inviteCount.Kirigami.FormData.checked ? inviteCount.value : -1)) {
						applicationWindow().showPassiveNotification(qsTr("Created invite"))
					} else {
						applicationWindow().showPassiveNotification(qsTr("Failed to create invite"))
					}
					createInvitesSheet.close()
				}
			}
		}
	}

	ListView {
		model: invitePage.inviteModel

		delegate: Kirigami.SwipeListItem {
			contentItem: ColumnLayout {
				QQC2.Label {
					text: `Invite ID: ${inviteID}`
					verticalAlignment: Text.AlignVCenter
				}
				QQC2.Label {
					text: possibleUses > 0 ? `Possible Uses: ${possibleUses} | Uses : ${uses} ` : `Possible Uses: Infinite | Uses : ${uses}`
					verticalAlignment: Text.AlignVCenter
				}
				anchors.verticalCenter: parent.verticalCenter
			}

			actions: [
				Kirigami.Action {
					icon.name: "edit-delete"
					onTriggered: invitePage.inviteModel.deleteInvite(inviteID)
				}
			]
		}
	}
}

