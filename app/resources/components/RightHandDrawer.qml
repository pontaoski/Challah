// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.10
import org.kde.kirigami 2.15 as Kirigami
import QtQml.Models 2.15
import com.github.HarmonyDevelopment.Challah 1.0

Control {
	leftPadding: 0
	rightPadding: 0
	topPadding: 0
	bottomPadding: 0

	Kirigami.Theme.inherit: false
	Kirigami.Theme.colorSet: Kirigami.Theme.View

	background: Rectangle {
		anchors.fill: parent

		Kirigami.Theme.inherit: false
		Kirigami.Theme.colorSet: Kirigami.Theme.View

		color: Kirigami.Theme.backgroundColor

		Kirigami.Separator {
			anchors {
				top: parent.top
				left: parent.left
				bottom: parent.bottom
			}
		}
	}
	contentItem: RowLayout {
		ListView {
			id: listView
			z: 1

			model: null
			Connections {
				id: conns
				target: routerInstance

				function update() {
					listView.model = CState.membersModelFor(routerInstance.guildHomeserver, routerInstance.guildID, this).valueOr(null)
				}
				function onGuildHomeserverChanged() {
					update()
				}
				function onGuildIDChanged() {
					update()
				}
			}

			Layout.fillHeight: true
			Layout.fillWidth: true
			Layout.preferredWidth: 200

			delegate: Kirigami.BasicListItem {
				id: del

				required property string userID

				text: userData.data.name
				reserveSpaceForSubtitle: true

				leading: Kirigami.Avatar {
					name: userData.data.name
					source: userData.data.avatarURL

					width: height
				}

				RelationalListener {
					id: userData

					model: CState.membersStore
					key: [routerInstance.guildHomeserver, del.userID]
					shape: QtObject {
						required property string name
						required property string avatarURL
					}
				}
			}
		}
	}
}
