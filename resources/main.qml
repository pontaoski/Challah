// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.ApplicationWindow {
	id: root

	minimumWidth: 500
	width: 1000

	pageStack.globalToolBar.showNavigationButtons: 0

	OverlappingPanels {
		anchors.fill: parent

		leftPanel: RightDrawer { id: rightHandDrawer }
		centerPanel: Kirigami.PageRow {
			id: colView
			implicitWidth: 400
			onImplicitWidthChanged: implicitWidth = 400
			columnView {
				columnResizeMode: Kirigami.ColumnView.SingleColumn
			}
			globalToolBar {
				style: Kirigami.ApplicationHeaderStyle.ToolBar
			}

			Kirigami.PageRouter {
				id: routerInstance

				initialRoute: "login"
				pageStack: colView.columnView

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
