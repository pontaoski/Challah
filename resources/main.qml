// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.ApplicationWindow {
	id: root

	minimumWidth: 400

	pageStack.globalToolBar.showNavigationButtons: 0

	OverlappingPanels {
		anchors.fill: parent

		leftPanel: RightDrawer { id: rightHandDrawer }
		centerPanel: Kirigami.ColumnView {
			id: colView
			implicitWidth: 400
			onImplicitWidthChanged: implicitWidth = 400
			columnResizeMode: Kirigami.ColumnView.SingleColumn

			Kirigami.PageRouter {
				id: routerInstance

				initialRoute: "login"
				pageStack: colView

				property var guildSheet: GuildSheet {
					id: guildSheet
				}

				LoginRoute {}
				NoGuildRoute {}
				ChannelRoute {}
				MessagesRoute {}
			}
		}
		rightPanel: StaccatoDrawer { id: leftHandDrawer }
	}

}
