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
	id: invitePage
	title: qsTr("Settings")
	padding: 0
	Kirigami.Theme.colorSet: Kirigami.Theme.View

	Kirigami.SwipeNavigator {
		anchors.fill: parent


		Kirigami.ScrollablePage {
			//: Alternate identities; e.g. for roleplay reasons
			title: qsTr("Identities")
			actions.main: Kirigami.Action {
				text: qsTr("New Identity")
				iconName: "list-add"
				onTriggered: personaAddSheet.begin()
			}

			Kirigami.OverlaySheet {
				id: personaAddSheet
				parent: root

				Kirigami.FormLayout {
					QQC2.TextField {
						id: nameField

						Kirigami.FormData.label: qsTr("Identity Name:")
					}
					QQC2.ComboBox {
						id: reasonField
						model: [
							{
								"uiString": qsTr("Roleplay"),
								"id": "roleplay"
							},
							{
								"uiString": "System Member",
								"id": "plurality"
							}
						]
						textRole: "uiString"
						Kirigami.FormData.label: qsTr("Identity Kind:")
					}
					QQC2.Button {
						text: qsTr("Create")
						onClicked: {
							let settings = uiSettings.personas
							settings.push({
								"name": nameField.text,
								"kind": reasonField.model[reasonField.currentIndex].id
							})
							uiSettings.personas = settings
							personaAddSheet.close()
						}
					}
				}

				function begin() {
					nameField.text = ""
					reasonField.currentIndex = 0
					this.open()
				}
			}

			ListView {
				model: uiSettings.personas
				delegate: Kirigami.SwipeListItem {
					id: personaDelegate

					contentItem: QQC2.Label {
						text: modelData["name"]
						verticalAlignment: Text.AlignVCenter
					}
					actions: [
						Kirigami.Action {
							icon.name: "edit-delete"
							onTriggered: {
								let a = uiSettings.personas
								print(index)
								a.splice(index)
								uiSettings.personas = a
							}
						}
					]
				}
			}
		}
	}
}
