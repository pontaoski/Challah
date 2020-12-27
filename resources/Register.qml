// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.Page {
	id: registerPage
	title: qsTr("Create an account")

	Kirigami.Theme.colorSet: Kirigami.Theme.View

	ColumnLayout {
		anchors.centerIn: parent

		Kirigami.FormLayout {
			QQC2.TextField {
				id: username
				//: placeholder for username
				placeholderText: qsTr("johndoe")
				Kirigami.FormData.label: qsTr("Username:")
			}
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
				text: qsTr("Create Account")
				onClicked: {
					registerPage.enabled = false
					HState.createAccount(username.text, email.text, password.text, homeserver.text, function(ok) {
						if (ok) {
							root.pageStack.layers.pop()
							root.showPassiveNotification(qsTr("Account created, welcome to Harmony!"))
						} else {
							root.showPassiveNotification(qsTr("Failed to create an account"))
						}
						registerPage.enabled = true
					})
				}
			}
		}
	}
}

