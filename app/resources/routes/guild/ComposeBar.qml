// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.15 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import QtQuick.Dialogs 1.3 as Dialogs
import com.github.HarmonyDevelopment.Challah 1.0

import "components" as Components
import "qrc:/components" as GlobalComponents

QQC2.ToolBar {
	id: composeRow

	function send() {
		timelineView.model.send(txtField.text, page.interactionID)
		txtField.text = ""
		page.interactionID = ""
		page.interactionKind = ""
	}

	Dialogs.FileDialog {
		id: fileDialog
		selectMultiple: true
		onAccepted: timelineView.model.sendFiles(this.fileUrls)
	}

	implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, contentItem.implicitWidth + leftPadding + rightPadding)
	implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, contentItem.implicitHeight + topPadding + bottomPadding)

	contentItem: ColumnLayout {
		QQC2.Control {
			visible: page.interactionID !== ""
			padding: 6
			contentItem: RowLayout {
				GlobalComponents.PlaintextMessage {
					id: interactionData

					messagesStore: timelineView.model.store
					messageID: page.interactionID
					homeserver: page.homeserver
				}
				Kirigami.Icon {
					source: page.interactionKind === "reply" ? "dialog-messages" : "edit-entry"
				}
				ColumnLayout {
					spacing: 1
					QQC2.Label {
						text: page.interactionKind == "edit" ? qsTr("Edit Message") : qsTr("Replying to <font color=\"%1\">%2</font>").arg(Kirigami.NameUtils.colorsFromString(interactionData.authorName)).arg(interactionData.authorName)
						elide: Text.ElideRight
						Layout.fillWidth: true
					}
					QQC2.Label {
						text: interactionData.onelinePlaintext
						elide: Text.ElideRight
						Layout.fillWidth: true
					}

					clip: true
					Layout.fillWidth: true
				}
				QQC2.ToolButton {
					icon.name: "dialog-cancel"
					onClicked: {
						page.interactionID = ""
						if (page.interactionKind == "edit") {
							txtField.clear()
						}
						page.interactionKind = ""
					}
				}
				Layout.fillWidth: true
			}
			Layout.fillWidth: true
		}
		RowLayout {
			QQC2.Button {
				Accessible.name: qsTr("Upload files")
				icon.name: "mail-attachment"
				onClicked: fileDialog.open()
			}
			// QQC2.Button {
			// 	text: "send test"
			// 	onClicked: {
			// 		timelineView.model.sendDebugMessage()
			// 		visible = false
			// 	}
			// }
			TextEdit {
				id: txtField

				activeFocusOnTab: true
				persistentSelection: true
				enabled: tryit(() => canSendPermissions.data.has, false)
				selectByMouse: !Kirigami.Settings.isMobile
				wrapMode: Text.Wrap

				color: Kirigami.Theme.textColor
				selectedTextColor: Kirigami.Theme.highlightedTextColor
				selectionColor: Kirigami.Theme.highlightColor

				QQC2.Label {
					visible: !txtField.text

					text: enabled ? qsTr("Write your message…") : qsTr("You cannot send messages.")

					opacity: 0.5
				}

				Keys.onReturnPressed: (event) => {
					if (!(event.modifiers & Qt.ShiftModifier)) {
						composeRow.send()
						event.accepted = true
					} else {
						event.accepted = false
					}
				}
				Keys.onTabPressed: nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
				Layout.fillWidth: true
			}
			QQC2.Button {
				Accessible.name: qsTr("Send message")
				icon.name: "document-send"
				onClicked: composeRow.send()
			}

			Layout.fillWidth: true
		}
	}
}
