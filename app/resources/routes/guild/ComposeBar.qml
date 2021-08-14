// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import QtQuick.Dialogs 1.3 as Dialogs
import com.github.HarmonyDevelopment.Challah 1.0

import "components" as Components

QQC2.ToolBar {
	id: composeRow

	function send() {
		timelineView.model.send(txtField.text)
		txtField.text = ""
	}

	Dialogs.FileDialog {
		id: fileDialog
		selectMultiple: true
		onAccepted: timelineView.model.sendFiles(this.fileUrls)
	}

	contentItem: ColumnLayout {
		RowLayout {
			QQC2.Button {
				Accessible.name: qsTr("Upload files")
				icon.name: "mail-attachment"
				onClicked: fileDialog.open()
			}
			QQC2.TextArea {
				id: txtField

				background: null
				wrapMode: Text.Wrap

				placeholderText: enabled ? qsTr("Write your message…") : qsTr("You cannot send messages.")

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
