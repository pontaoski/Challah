// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Challah 1.0

Kirigami.ScrollablePage {
	id: pageRoot

    titleDelegate: RowLayout {
        QQC2.ToolButton {
            icon.name: "arrow-left"
            onClicked: colView.layers.pop()
        }
        Kirigami.Heading {
            text: qsTr("Invites")
            level: 4
        }
        Item { Layout.fillWidth: true }
    }

    QQC2.RoundButton {
        icon.name: "list-add"
        text: qsTr("New Invite")
        onClicked: createInvitesSheet.open()

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: Kirigami.Units.largeSpacing
    }

	property var invitesModel: CState.inviteModelFor(routerInstance.guildHomeserver, routerInstance.guildID, this)

	Kirigami.Theme.colorSet: Kirigami.Theme.View

	Kirigami.OverlaySheet {
		id: createInvitesSheet

		parent: QQC2.Overlay.overlay

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
					pageRoot.
						invitesModel.
						createInvite(inviteField.text, inviteCount.Kirigami.FormData.checked ? inviteCount.value : -1).
						then((ok) => {
							if (ok) {
								applicationWindow().showPassiveNotification(qsTr("Created invite"))
							} else {
								applicationWindow().showPassiveNotification(qsTr("Failed to create invite"))
							}
							createInvitesSheet.close()
						})
				}
			}
		}
	}

	ListView {
		model: pageRoot.invitesModel

		delegate: Kirigami.SwipeListItem {
            alwaysVisibleActions: true

			contentItem: ColumnLayout {
				QQC2.Label {
					text: qsTr("Invite ID: %1").arg(name)
					verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
				}
				QQC2.Label {
					text: possibleUses > 0 ? qsTr("Possible Uses: %1 | Uses: %2 ").arg(possibleUses).arg(uses) : qsTr("Possible Uses: Infinite | Uses : %1").arg(uses)
					verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
				}
				anchors.verticalCenter: parent.verticalCenter
			}

			actions: [
				Kirigami.Action {
					icon.name: "edit-delete"
					onTriggered: pageRoot.invitesModel.deleteInvite(name)
				}
			]
		}
	}
}

