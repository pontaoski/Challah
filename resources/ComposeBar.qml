// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

QQC2.ToolBar {
	position: QQC2.ToolBar.Footer

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
					Kirigami.PageRouter.data.sendMessage(text, replyingBar.replyingToID)
					text = ""
					replyingBar.replyingToID = ""
				}

				Keys.onEscapePressed: replyingBar.replyingToID = ""

				onAccepted: send()
			}
			QQC2.Button {
				//: Send the message in the text field
				text: qsTr("Send")
				onClicked: messageField.send()
			}
		}
	}
}
