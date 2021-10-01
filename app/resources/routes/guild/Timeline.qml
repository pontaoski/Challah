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

	property string interactionID: ""
	property string interactionKind: ""

	footer: ComposeBar {
		id: composeRow
	}

	RelationalListener {
		id: canDeletePermissions

		model: CState.ownPermissionsStore
		key: [routerInstance.params.homeserver, routerInstance.params.guildID, routerInstance.params.channelID, "messages.manage.delete"]
		shape: QtObject {
			required property bool has
		}
	}
	RelationalListener {
		id: canSendPermissions

		model: CState.ownPermissionsStore
		key: [routerInstance.params.homeserver, routerInstance.params.guildID, routerInstance.params.channelID, "messages.send"]
		shape: QtObject {
			required property bool has
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
