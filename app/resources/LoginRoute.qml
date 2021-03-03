// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Layouts 1.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.PageRoute {
	name: "login"
	cache: false

	Kirigami.Page {
		title: qsTr("Welcome")

		Kirigami.Theme.colorSet: Kirigami.Theme.View

		ColumnLayout {
			anchors.centerIn: parent

			Image {
				source: "qrc:/img/io.harmonyapp.Challah.svg"
				sourceSize {
					width: Layout.preferredWidth
					height: Layout.preferredWidth
				}

				Layout.preferredWidth: Kirigami.Units.gridUnit * 7
				Layout.preferredHeight: Layout.preferredWidth

				Layout.alignment: Qt.AlignHCenter
			}

			Kirigami.Heading {
				text: qsTr("Welcome to Challah")
				horizontalAlignment: Text.AlignHCenter

				Layout.fillWidth: true
			}
			Kirigami.Heading {
				text: qsTr("Enter your homeserver to continue")
				horizontalAlignment: Text.AlignHCenter
				level: 4

				Layout.fillWidth: true
			}

			Item { implicitHeight: Kirigami.Units.largeSpacing }

			QQC2.TextField {
				id: homeserver
				placeholderText: "chat.harmonyapp.io:2289"

				objectName: "LoginRoute-Homeserver-TextField"

				Layout.fillWidth: true
			}

			QQC2.Button {
				text: qsTr("Continue")

				objectName: "LoginRoute-Homeserver-Continue"
				enabled: homeserver.text !== "" && homeserver.text.includes(":")

				Layout.alignment: Qt.AlignHCenter

				onClicked: {
					let manager = root.pageStack.layers.push(Qt.resolvedUrl("Stepper.qml")).manager
					manager.beginLogin(homeserver.text)
				}
			}
		}
	}
}
