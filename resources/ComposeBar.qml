// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import QtQuick.Dialogs 1.3
import com.github.HarmonyDevelopment.Staccato 1.0

QQC2.ToolBar {
	position: QQC2.ToolBar.Footer

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

				QQC2.TextField {
					id: dialogField
					//: Placeholder text for the message field
					placeholderText: qsTr("Write a message...")

					Layout.fillWidth: true

					function send() {
						messageField.Kirigami.PageRouter.data.sendMessage(text, replyingBar.replyingToID, [messageField.Kirigami.PageRouter.data.uploadFile(uploadSheet.pendingUpload)])
						text = ""
						replyingBar.replyingToID = ""
					}

					onAccepted: send()
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
		QQC2.Control {
			id: replyingBar
			visible: replyingToID !== ""
			property string replyingToID: ""
			property string replyingToAuthor: ""
			property string replyingToContent: ""

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

						Layout.fillWidth: true
					}
					Layout.fillWidth: true
				}
				QQC2.ToolButton {
					flat: true
					icon.name: "dialog-close"
					onClicked: replyingBar.replyingToID = ""
				}
			}

			Layout.fillWidth: true
		}
		RowLayout {
			QQC2.TextField {
				id: messageField
				//: Placeholder text for the message field
				placeholderText: qsTr("Write a message...")

				Layout.fillWidth: true

				function send() {
					Kirigami.PageRouter.data.sendMessage(text, replyingBar.replyingToID, [])
					text = ""
					replyingBar.replyingToID = ""
				}
				background: null

				Keys.onEscapePressed: replyingBar.replyingToID = ""

				onAccepted: send()
			}
			QQC2.Button {
				icon.name: "mail-attachment"
				hoverEnabled: true
				onClicked: fileDialog.open()

				QQC2.ToolTip.delay: Kirigami.Units.shortDuration
				QQC2.ToolTip.visible: hovered
				//: Send the message in the text field
				QQC2.ToolTip.text: qsTr("Add attachment...")
			}
			QQC2.Button {
				icon.name: "document-send"
				hoverEnabled: true
				onClicked: messageField.send()

				QQC2.ToolTip.delay: Kirigami.Units.shortDuration
				QQC2.ToolTip.visible: hovered
				//: Send the message in the text field
				QQC2.ToolTip.text: qsTr("Send")
			}
		}
	}
}
