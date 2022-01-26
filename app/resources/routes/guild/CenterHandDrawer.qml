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

	implicitWidth: 400

	RelationalListener {
		id: channelData

		model: CState.channelsModelFor(page.homeserver, page.guildID, this).store
		key: page.channelID
		shape: QtObject {
			required property string name
			required property string kind
		}
	}

	header: GlobalComponents.Header {
		ColumnLayout {
			QQC2.Label {
				text: tryit(() => `#${channelData.data.name}`, "Channel")

				Layout.fillWidth: true
			}
			RowLayout {
				TypingDots { visible: timelineView.model.nowTyping.length > 0 }
				QQC2.Label {
					text: timelineView.model.nowTyping.length === 0 ?
						qsTr("Nobody is typing") :
						qsTr("%1 is typing...", "", timelineView.model.nowTyping.length).arg(
						UIUtils.naturalList(
							timelineView.model.nowTyping.map((item) =>
								CState.membersStore.data([routerInstance.guildHomeserver, CState.ownID], 0) || "")))
					opacity: timelineView.model.nowTyping.length == 0 ? 0.4 : 1.0
					color: timelineView.model.nowTyping.length == 0 ?
						Kirigami.Theme.textColor :
						Kirigami.Theme.focusColor

					font: Kirigami.Theme.smallFont

					Layout.fillWidth: true
				}

				Layout.fillWidth: true
			}

			Layout.margins: 8
			Layout.fillWidth: true
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

	Component {
		id: voiceInformation

		QQC2.ToolBar {
			id: voiceInformationBar
			property var voiceCall: null

			anchors {
				top: parent.top
				left: parent.left
				right: parent.right
			}

			contentItem: RowLayout {
				Item { Layout.fillWidth: true }
				QQC2.Button {
					text: "Connect"
					onClicked: voiceInformationBar.voiceCall = CState.makeVoiceCall(page.homeserver, page.guildID, page.channelID, this)
				}
			}
		}
	}

	ListView {
		id: timelineView

		model: CState.messagesModelFor(page.homeserver, page.guildID, page.channelID, this)
		// TODO: bugs
		// reuseItems: true
		verticalLayoutDirection: ListView.BottomToTop
		activeFocusOnTab: true

		delegate: Components.MessageDelegate {}

		Loader {
			anchors {
				top: parent.top
				left: parent.left
				right: parent.right
			}
			sourceComponent: {
				switch (channelData.data.kind) {
				case "voice":
					return null // voiceInformation
				default:
					return null
				}
			}
		}
	}
}
