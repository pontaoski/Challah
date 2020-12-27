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
		title: qsTr("Login")

		Kirigami.Theme.colorSet: Kirigami.Theme.View

		ColumnLayout {
			anchors.centerIn: parent

			Image {
				source: "qrc:/img/io.harmonyapp.Murmur.svg"
				sourceSize {
					width: Layout.preferredWidth
					height: Layout.preferredWidth
				}

				Layout.preferredWidth: Kirigami.Units.gridUnit * 7
				Layout.preferredHeight: Layout.preferredWidth

				Layout.alignment: Qt.AlignHCenter
			}

			Kirigami.Heading {
				text: qsTr("Welcome to Murmur")
				horizontalAlignment: Text.AlignHCenter

				Layout.fillWidth: true
			}

			Kirigami.FormLayout {
				QQC2.TextField {
					id: email
					//: placeholder for email
					placeholderText: qsTr("email@address.com")
					// text: "r@r.r"
					Kirigami.FormData.label: qsTr("Email:")
				}
				QQC2.TextField {
					id: homeserver
					placeholderText: "chat.harmonyapp.io:2289"
					// text: "localhost:2289"
					Kirigami.FormData.label: qsTr("Homeserver:")
				}
				Kirigami.PasswordField {
					id: password
					// text: "10kekeAke"
					Kirigami.FormData.label: qsTr("Password:")
				}
				QQC2.Button {
					text: qsTr("Login")
					onClicked: {
						if (HState.login(email.text, password.text, homeserver.text)) {
							//: the user logged in successfully
							root.showPassiveNotification(qsTr("Logged in"))
						} else {
							//: login failed
							root.showPassiveNotification(qsTr("Failed to log in"))
						}
					}
				}
			}
			Item { implicitHeight: Kirigami.Units.gridUnit }
			Kirigami.LinkButton {
				text: qsTr("Don't have an account yet?")
				onClicked: root.pageStack.layers.push(Qt.resolvedUrl("Register.qml"))

				Layout.fillWidth: true
			}
		}
	}
}
