// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import com.github.HarmonyDevelopment.Challah 1.0
import QtGraphicalEffects 1.12

import QtQuick.Controls 2.12 as QQC2

import "qrc:/routes/login" as LoginRoutes
import "qrc:/routes/guild" as GuildRoutes
import "qrc:/routes/settings" as SettingsRoutes
import "qrc:/components" as Components

Kirigami.ApplicationWindow {
	id: root

	property bool testing: false

	minimumWidth: 300
	width: 1000

	wideScreen: width > 500

	Loader {
		id: settingsLoader
		asynchronous: true
		active: false

		sourceComponent: SettingsRoutes.Settings {
			visible: false
		}
	}

	function tryit(fn, def) {
		try {
			return fn() ?? def
		} catch (error) {
			return def
		}
	}

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

	Kirigami.PageRow {
		id: colView
		anchors.fill: parent
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
				settingsLoader.active = true
				routerInstance.navigateToRoute("Guild/Timeline")
			}
		}

		Kirigami.PageRouter {
			id: routerInstance

			initialRoute: "loading"
			pageStack: colView.columnView

			Component.onCompleted: CState.doInitialLogin()

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
		}

		Kirigami.Heading {
			level: 4

			visible: CState.connectionStatus != CState.Connected

			text: {
				switch (CState.connectionStatus) {
				case CState.Offline:
					return qsTr("Offline")
				case CState.PartialOutage:
					return qsTr("Partial Outage")
				case CState.Connecting:
					return qsTr("Connecting…")
				case CState.Connected:
					return qsTr("Connected")
				}
			}

			padding: Kirigami.Units.smallSpacing
			leftPadding: Kirigami.Units.largeSpacing
			rightPadding: Kirigami.Units.largeSpacing

			anchors.bottom: parent.bottom
			anchors.left: parent.left
			anchors.margins: 6

			z: 99

			background: Rectangle {
				radius: height

				Kirigami.Theme.colorSet: Kirigami.Theme.Window
				color: Kirigami.Theme.backgroundColor

				layer.enabled: true
				layer.effect: DropShadow {
					cached: true
					horizontalOffset: 0
					verticalOffset: 1
					radius: 2.0
					samples: 17
					color: "#30000000"
				}
			}
		}
	}
}
