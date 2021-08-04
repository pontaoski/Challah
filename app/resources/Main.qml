// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import com.github.HarmonyDevelopment.Challah 1.0

import QtQuick.Controls 2.12 as QQC2

import "qrc:/routes/login" as LoginRoutes
import "qrc:/routes/guild" as GuildRoutes
import "qrc:/components" as Components

Kirigami.ApplicationWindow {
	id: root

	property bool testing: false

	minimumWidth: 300
	width: 1000

	wideScreen: width > 500

	UISettings { id: uiSettings }

	Window {
		id: textAsker

		flags: Qt.Sheet
		modality: Qt.WindowModal

		title: "\u2800"

		property var promise: null

		function ask(text) {
			const obj = {
				"cb": (it) => it,
				"then": function(then) {
					this.cb = then
				}
			}
			promise = obj
			textLabel.text = text
			this.visible = true
			return obj
		}

		color: Kirigami.Theme.backgroundColor

		Kirigami.Separator {
			anchors {
				top: parent.top
				left: parent.left
				right: parent.right
			}
		}

		width: askerCon.implicitWidth
		maximumWidth: askerCon.implicitWidth
		minimumWidth: askerCon.implicitWidth
		height: askerCon.implicitHeight
		maximumHeight: askerCon.implicitHeight
		minimumHeight: askerCon.implicitHeight

		QQC2.Control {
			id: askerCon
			padding: Kirigami.Units.gridUnit * 2
			contentItem: ColumnLayout {
				QQC2.Label {
					id: textLabel
				}
				RowLayout {
					Layout.fillWidth: true

					QQC2.TextField {
						id: textEntry
						onAccepted: {
							textAsker.promise.cb(text)
							textAsker.visible = false
						}

						Layout.fillWidth: true
					}
					QQC2.Button {
						icon.name: "arrow-right"
						onClicked: {
							textAsker.promise.cb(textEntry.text)
							textAsker.visible = false
						}
					}
				}
			}
		}
	}

	pageStack.globalToolBar.showNavigationButtons: 0
	component PG : Kirigami.Page {}
	pageStack.initialPage: PG {
		padding: 0
		globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None
		Kirigami.Theme.colorSet: Kirigami.Theme.View

		OverlappingPanels {
			anchors.fill: parent

			leftPanel: Loader {
				id: leftLoader

				Kirigami.PageRouter.router: routerInstance
				Kirigami.PageRouter.watchedRoute: ["Guild/Blank"]
				active: Kirigami.PageRouter.watchedRouteActive

				sourceComponent: Components.LeftHandDrawer {
				}
			}
			rightPanel: Loader {
				id: rightLoader

				active: routerInstance.guildID != ""

				sourceComponent: Components.RightHandDrawer {
				}
			}
			centerPanel: Kirigami.PageRow {
				id: colView
				implicitWidth: 400
				onImplicitWidthChanged: implicitWidth = 400
				clip: true
				columnView {
					columnResizeMode: Kirigami.ColumnView.SingleColumn
					interactive: false
				}
				globalToolBar {
					style: Kirigami.ApplicationHeaderStyle.ToolBar
				}

				Connections {
					target: CState


					function onBeginHomeserver() {
						routerInstance.navigateToRoute("Login/HomeserverPrompt")
					}
					function onBeginLogin() {
						routerInstance.navigateToRoute("Login/Stepper")
					}
					function onEndLogin() {
						routerInstance.navigateToRoute("Guild/Blank")
					}
				}

				Kirigami.PageRouter {
					id: routerInstance

					initialRoute: "loading"
					pageStack: colView.columnView

					Component.onCompleted: CState.doInitialLogin()
					onNavigationChanged: {
						leftLoader.active = leftLoader.Kirigami.PageRouter.watchedRouteActive
					}

					property var guildSheet: GuildSheet {
						id: guildSheet
					}
					property string guildHomeserver
					property string guildID
					property string channelID

					LoadingRoute {}
					LoginRoutes.Homeserver {}
					LoginRoutes.Stepper {}

					GuildRoutes.Blank {}
					GuildRoutes.Timeline {}

					// LoginRoute {}
					// GuildRoute {}
					// NoGuildRoute {}
					// MessagesRoute {}
				}
			}
		}
	}

}
