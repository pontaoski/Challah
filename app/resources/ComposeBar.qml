// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import QtQuick.Dialogs 1.3
import com.github.HarmonyDevelopment.Challah 1.0

QQC2.ToolBar {
	id: composeBar

	position: QQC2.ToolBar.Footer
	property alias replies: replyingBar

	property var model: HState.messagesModel(
		Kirigami.PageRouter.router.params.guildID,
		Kirigami.PageRouter.router.params.channelID,
		Kirigami.PageRouter.router.params.homeserver,
	)

	FileDialog {
		id: fileDialog
		title: "Please choose a file"
		folder: shortcuts.pictures
		onAccepted: {
			uploadSheet.pendingUpload = fileDialog.fileUrl
			uploadSheet.open()
		}
	}

	Kirigami.OverlaySheet {
		id: uploadSheet
		property string pendingUpload
		parent: root

		ColumnLayout {
			Image {
				source: uploadSheet.pendingUpload
				fillMode: Image.PreserveAspectCrop
				smooth: true
				mipmap: true

				Layout.preferredWidth: Kirigami.Settings.isMobile ? root.width * (2/3) : root.width * (1/8)
				Layout.preferredHeight: Layout.preferredWidth
				Layout.alignment: Qt.AlignHCenter
			}

			RowLayout {
				Layout.fillWidth: true

				QQC2.TextArea {
					id: dialogField
					//: Placeholder text for the message field
					placeholderText: qsTr("Write a message...")

					Layout.fillWidth: true

					function send() {
						let incomingText = text
						let replyingTo = replyingBar.replyingToID
						text = ""
						replyingBar.replyingToID = ""
						composeBar.model.parentModel.uploadFile(
							uploadSheet.pendingUpload,
							function(url) {
								composeBar.model.sendMessage(incomingText, replyingTo, [url])
							},
							function() {
								root.showPassiveNotification(qsTr("Failed to upload file"))
							},
							function(progress) {},
							function() {}
						)
						uploadSheet.close()
					}

					Keys.onReturnPressed: send()

					Component.onCompleted: HState.bindTextDocument(this.textDocument, this)
				}
				QQC2.Button {
					text: qsTr("Send")
					onClicked: dialogField.send()
				}
			}
		}
	}

	ColumnLayout {
		anchors {
			left: parent.left
			right: parent.right
		}
		spacing: 0
		Expandable {
			childVisible: replyingBar.replyingToID !== ""

			QQC2.Control {
				id: replyingBar
				property string replyingToID: ""
				property string replyingToAuthor: ""
				property string replyingToContent: ""

				anchors.left: parent.left
				anchors.right: parent.right

				padding: Kirigami.Units.gridUnit
				contentItem: RowLayout {
					Kirigami.Icon {
						source: "dialog-messages"
					}
					ColumnLayout {
						Kirigami.Heading {
							level: 3
							text: replyingBar.replyingToAuthor
							textFormat: TextEdit.MarkdownText
						}
						QQC2.Label {
							text: replyingBar.replyingToContent
							textFormat: TextEdit.MarkdownText
							clip: true

							Layout.fillWidth: true
						}
						Layout.fillWidth: true
					}
					QQC2.ToolButton {
						flat: true
						icon.name: "dialog-close"
						onClicked: replyingBar.replyingToID = ""
					}

					Layout.fillWidth: true
				}
			}

			Layout.fillWidth: true
		}
		RowLayout {
			QQC2.ComboBox {
				id: settingsCombo
				model: [{
					"name": qsTr("Default"),
					"kind": "default"
				}].concat(uiSettings.personas)
				visible: uiSettings.personas.length > 0
				textRole: "name"
			}
			QQC2.Button {
				icon.name: "mail-attachment"
				enabled: messagesRoute.model.permissions.canSendAndEdit
				hoverEnabled: true
				onClicked: fileDialog.open()

				QQC2.ToolTip.delay: Kirigami.Units.shortDuration
				QQC2.ToolTip.visible: hovered
				//: Send the message in the text field
				QQC2.ToolTip.text: qsTr("Add attachment...")

				Layout.fillHeight: true
				Layout.preferredWidth: height
			}
			QQC2.TextArea {
				id: messageField
				placeholderText: if (messagesRoute.model.permissions.canSendAndEdit) {
					//: Placeholder text for the message field
					return qsTr("Write a message...")
				} else {
					//: Placeholder text for the message field when the user isn't allowed to send a message
					return qsTr("You do not have permissions to send a message to this channel.")
				}
				onTextChanged: {
					model.typed()
				}

				padding: 8

				Clipboard.paste: function(clipboard) {
					if (clipboard.hasUrls) {
						uploadSheet.pendingUpload = clipboard.urls[0]
						uploadSheet.open()
						return true
					}
				}
				Component.onCompleted: HState.bindTextDocument(this.textDocument, this)
				Layout.fillWidth: true

				function send() {
					if (text == "") {
						return
					}
					if (uiSettings.personas.length == 0) {
						model.sendMessage(text, replyingBar.replyingToID, [])
					} else {
						switch (settingsCombo.model[settingsCombo.currentIndex].kind) {
						case "default":
							model.sendMessage(text, replyingBar.replyingToID, []); break
						case "roleplay":
							model.sendMessageAsRoleplay(text, replyingBar.replyingToID, [], settingsCombo.model[settingsCombo.currentIndex].name); break
						case "plurality":
							model.sendMessageAsSystem(text, replyingBar.replyingToID, [], settingsCombo.model[settingsCombo.currentIndex].name); break
						}
					}
					text = ""
					replyingBar.replyingToID = ""
				}
				background: null

				Keys.onEscapePressed: replyingBar.replyingToID = ""
				Keys.onReturnPressed: send()
			}
			QQC2.Button {
				icon.name: "document-send"
				enabled: messagesRoute.model.permissions.canSendAndEdit
				hoverEnabled: true
				onClicked: messageField.send()

				QQC2.ToolTip.delay: Kirigami.Units.shortDuration
				QQC2.ToolTip.visible: hovered
				//: Send the message in the text field
				QQC2.ToolTip.text: qsTr("Send")

				Layout.fillHeight: true
				Layout.preferredWidth: height
			}
		}
	}
}
