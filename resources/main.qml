// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.ApplicationWindow {
	id: root

	minimumWidth: 300
	width: 1000

	pageStack.globalToolBar.showNavigationButtons: 0

	QtObject {
		id: fauxApplicationWindow
		property alias pageStack: colView
		property alias globalDrawer: root.globalDrawer
		property alias contextDrawer: root.contextDrawer
	}

	function applicationWindow() {
		return fauxApplicationWindow
	}

	OverlappingPanels {
		anchors.fill: parent

		leftPanel: StaccatoDrawer { id: leftHandDrawer }
		rightPanel: RightDrawer { id: rightHandDrawer }
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

			Connections {
				target: HState
				function onLoggedIn() {
					routerInstance.navigateToRoute("no-guild")
					leftHandDrawer.shouldShow = true
				}
			}

			Kirigami.PageRouter {
				id: routerInstance

				initialRoute: {
					if (HState.startupLogin()) {
						leftHandDrawer.shouldShow = true
						return "no-guild"
					} else {
						return "login"
					}
				}
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
	}

}
