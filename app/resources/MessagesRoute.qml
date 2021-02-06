// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0
import QtQml.Models 2.15

Kirigami.PageRoute {
	name: "messages"
	cache: true

	Kirigami.ScrollablePage {
		id: messagesRoute
		Kirigami.Theme.colorSet: Kirigami.Theme.View

		property var model: HState.messagesModel(
			Kirigami.PageRouter.router.params.guildID,
			Kirigami.PageRouter.router.params.channelID,
			Kirigami.PageRouter.router.params.homeserver,
		)

		header: Expandable {
			childVisible: messagesRoute.model.typingIndicator
			direction: Expandable.Direction.FromTop

			QQC2.Control {
				background: Rectangle {
					Kirigami.Theme.inherit: false
					Kirigami.Theme.colorSet: Kirigami.Theme.Header
					color: Kirigami.Theme.backgroundColor

					Kirigami.Separator {
						anchors {
							bottom: parent.bottom
							left: parent.left
							right: parent.right
						}
					}
				}
				padding: Kirigami.Units.largeSpacing
				contentItem: QQC2.Label {
					text: messagesRoute.model.typingIndicator
				}
				anchors.left: parent.left
				anchors.right: parent.right
			}
		}
		footer: ComposeBar { id: composeBar }

		ItemSelectionModel {
			id: messagesSelectionModel
			model: messagesView.model
		}

		ListView {
			id: messagesView
			model: messagesRoute.model
			verticalLayoutDirection: ListView.BottomToTop
			bottomMargin: 0
			topMargin: 0
			leftMargin: Kirigami.Units.gridUnit
			rightMargin: Kirigami.Units.gridUnit
			spacing: 0

			section {
				criteria: ViewSection.FullString
				property: "authorIDAndAvatar"
				delegate: Item {
					required property var section

					Kirigami.Avatar {
						id: avvy

						name: parent.section.split("\t")[2]
						source: parent.section.split("\t")[1]

						width: Kirigami.Units.gridUnit * 2
						height: Kirigami.Units.gridUnit * 2

						visible: !(avvy.parent.section.split("\t")[0] === messagesView.model.userID() && (!applicationWindow().wideScreen))

						anchors {
							bottom: parent.top
						}

						UserPopup {
							id: popup
							name: avvy.name
							source: avvy.source
							userID: avvy.parent.section.split("\t")[0]
						}

						actions.main: Kirigami.Action {
							onTriggered: popup.begin()
						}
					}

					width: 0
					height: 0
				}
			}

			delegate: ColumnLayout {
				MessageDelegate {
					Layout.alignment: (isOwnMessage && (!applicationWindow().wideScreen)) ? Qt.AlignRight : Qt.AlignLeft
				}
				anchors.left: parent.left
				anchors.right: parent.right

				Layout.fillWidth: true
			}

			add: Transition {
				NumberAnimation {
					duration: Kirigami.Units.shortDuration
					properties: "y"
					easing.type: Easing.InOutCubic
				}
			}
			displaced: Transition {
				NumberAnimation {
					duration: Kirigami.Units.shortDuration
					properties: "y"
					easing.type: Easing.InOutCubic
				}
			}
		}
	}
}
