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
		title: "Login"

		Kirigami.Theme.colorSet: Kirigami.Theme.View

		ColumnLayout {
			anchors.centerIn: parent

			Image {
				source: "qrc:/img/com.github.Harmony.Murmur.svg"
				sourceSize {
					width: Layout.preferredWidth
					height: Layout.preferredWidth
				}

				Layout.preferredWidth: Kirigami.Units.gridUnit * 7
				Layout.preferredHeight: Layout.preferredWidth

				Layout.alignment: Qt.AlignHCenter
			}

			Kirigami.Heading {
				text: "Welcome to Murmur"
				horizontalAlignment: Text.AlignHCenter

				Layout.fillWidth: true
			}

			Kirigami.FormLayout {
				QQC2.TextField {
					id: email
					placeholderText: "email@address.com"
					// text: "r@r.r"
					Kirigami.FormData.label: "Email:"
				}
				QQC2.TextField {
					id: homeserver
					placeholderText: "harmonyapp.io:2289"
					// text: "localhost:2289"
					Kirigami.FormData.label: "Homeserver:"
				}
				Kirigami.PasswordField {
					id: password
					// text: "10kekeAke"
					Kirigami.FormData.label: "Password:"
				}
				QQC2.Button {
					text: "Login"
					onClicked: {
						if (HState.login(email.text, password.text, homeserver.text)) {
							root.showPassiveNotification("Logged in")
						} else {
							root.showPassiveNotification("Failed to log in")
						}
					}
				}
			}
		}
	}
}
