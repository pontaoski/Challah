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

	RelationalListener {
		id: canSendPermissions

		model: CState.ownPermissionsStore
		key: [routerInstance.params.homeserver, routerInstance.params.guildID, routerInstance.params.channelID, "messages.send"]
		shape: QtObject {
			required property bool has
		}
	}

	Dialogs.FileDialog {
		id: fileDialog
		selectMultiple: true
		onAccepted: timelineView.model.sendFiles(this.fileUrls)
	}

	implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, contentItem.implicitWidth + leftPadding + rightPadding)
	implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, contentItem.implicitHeight + topPadding + bottomPadding)

	contentItem: RowLayout {
		QQC2.Button {
			Accessible.name: qsTr("Upload files")
			icon.name: "mail-attachment"
			onClicked: fileDialog.open()
		}
		TextEdit {
			id: txtField

			activeFocusOnTab: true
			persistentSelection: true
			enabled: canSendPermissions.data.has
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
