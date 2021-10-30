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
import "qrc:/components" as GlobalComponents

Kirigami.ScrollablePage {
	id: page

	globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None

	Kirigami.Theme.colorSet: Kirigami.Theme.View

	property string homeserver: routerInstance.guildHomeserver
	property string guildID: routerInstance.guildID
	property string channelID: routerInstance.channelID


	RelationalListener {
		id: channelData

		model: CState.channelsModelFor(page.homeserver, page.guildID, this).store
		key: page.channelID
		shape: QtObject {
			required property string name
		}
	}

	header: GlobalComponents.Header {
		RowLayout {
			Kirigami.Heading {
				level: 4
				text: tryit(() => `#${channelData.data.name}`, "Channel")

				Layout.fillWidth: true
			}

			Layout.margins: 8
		}
	}

	property string interactionID: ""
	property string interactionKind: ""

	footer: ComposeBar {
		id: composeRow
	}

	RelationalListener {
		id: canDeletePermissions

		model: CState.ownPermissionsStore
		key: [page.homeserver, page.guildID, page.channelID, "messages.manage.delete"]
		shape: QtObject {
			required property bool has
		}
	}
	RelationalListener {
		id: canSendPermissions

		model: CState.ownPermissionsStore
		key: [page.homeserver, page.guildID, page.channelID, "messages.send"]
		shape: QtObject {
			required property bool has
		}
	}

	ListView {
		id: timelineView

		model: CState.messagesModelFor(page.homeserver, page.guildID, page.channelID, this)
		reuseItems: true
		verticalLayoutDirection: ListView.BottomToTop
		activeFocusOnTab: true

		delegate: Components.MessageDelegate {}
	}
}
