// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.ApplicationWindow {
	id: root

	property bool testing: false

	minimumWidth: 300
	width: 1000

	wideScreen: width > 500

	UISettings { id: uiSettings }

	pageStack.globalToolBar.showNavigationButtons: 0
	pageStack.initialPage: Kirigami.Page {
		padding: 0
		globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None
		Kirigami.Theme.colorSet: Kirigami.Theme.View

		OverlappingPanels {
			anchors.fill: parent

			leftPanel: StaccatoDrawer { id: leftHandDrawer }
			rightPanel: RightDrawer { id: rightHandDrawer }
			centerPanel: Kirigami.PageRow {
				id: colView
				implicitWidth: 400
				onImplicitWidthChanged: implicitWidth = 400
				clip: true
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
					function onLoggedOut() {
						routerInstance.navigateToRoute("login")
						leftHandDrawer.shouldShow = false
						rightHandDrawer.model = null
					}
				}

				Kirigami.PageRouter {
					id: routerInstance

					initialRoute: {
						if (root.testing) return "login"

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
					GuildRoute {}
					NoGuildRoute {}
					MessagesRoute {}
				}
			}
		}
	}

}
