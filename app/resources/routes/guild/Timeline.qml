// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Challah 1.0

import "components" as Components

Kirigami.PageRoute {
name: "Guild/Timeline"
cache: true

Kirigami.ScrollablePage {
	id: page

	globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None

	Kirigami.Theme.colorSet: Kirigami.Theme.View

	property string homeserver
	property string guildID
	property string channelID

	footer: QQC2.ToolBar {
		contentItem: ColumnLayout {
			RowLayout {
				id: composeRow

				function send(text) {
					timelineView.model.send(txtField.text)
					txtField.text = ""
				}

				QQC2.TextArea {
					id: txtField

					background: null
					wrapMode: Text.Wrap

					placeholderText: enabled ? qsTr("Write your message…") : qsTr("You cannot send messages.")

					Keys.onReturnPressed: (event) => {
						if (!(event.modifiers & Qt.ShiftModifier)) {
							composeRow.send(txtField.text)
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
				}

				Layout.fillWidth: true
			}
		}
	}

	ListView {
		id: timelineView

		Component.onCompleted: model = CState.messagesModelFor(page.homeserver, page.guildID, page.channelID, this).valueOr(null)

		reuseItems: true
		verticalLayoutDirection: ListView.BottomToTop
		activeFocusOnTab: true

		delegate: Components.MessageDelegate {}
	}
}

}
