// SPDX-FileCopyrightText: 2022 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import QtQuick.Dialogs 1.3 as Dialogs
import QtQml.Models 2.15
import com.github.HarmonyDevelopment.Challah 1.0

Kirigami.Page {
	id: pageRoot

    titleDelegate: RowLayout {
        QQC2.ToolButton {
            icon.name: "arrow-left"
            onClicked: colView.layers.pop()
        }
        Kirigami.Heading {
            text: qsTr("Edit Guild Information")
            level: 4
        }
        Item { Layout.fillWidth: true }
    }

	RelationalListener {
		id: canUpdateInfo

		model: CState.ownPermissionsStore
		key: [routerInstance.guildHomeserver, routerInstance.guildID, 0, "guild.manage.change-information"]
		shape: QtObject {
			required property bool has
		}
	}

	RelationalListener {
		id: guildData

		model: CState.guildsStore
		key: [routerInstance.guildHomeserver, routerInstance.guildID]
		shape: QtObject {
			required property string name
			required property string picture
		}
	}

	ColumnLayout {
		anchors.fill: parent

		ColumnLayout {
			Layout.maximumWidth: 400
			Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

			Kirigami.Heading {
				text: tryit(() => guildData.data.name, "")

				Layout.fillWidth: true
			}

			QQC2.Button {
                visible: !nameField.isChangingName

				text: qsTr("Change Name")
				enabled: tryit(() => canUpdateInfo.data.has, false)
				onClicked: {
                    nameField.isChangingName = true
                    nameField.forceActiveFocus()
                }
			}

            RowLayout {
                visible: nameField.isChangingName

                Layout.fillWidth: true

                QQC2.TextField {
                    id: nameField

                    property bool isChangingName: false
                    function changeName() {
                        CState.guildsStore.setName(routerInstance.guildHomeserver, routerInstance.guildID, this.text)
                        this.isChangingName = false
                    }

                    Layout.fillWidth: true
                }
                QQC2.ToolButton {
                    icon.name: "dialog-ok"
                    onClicked: nameField.changeName()
                }
            }

			Kirigami.Avatar {
				implicitWidth: 100
				implicitHeight: 100

				name: tryit(() => guildData.data.name, "")
				source: tryit(() => guildData.data.picture, "")
			}

			Dialogs.FileDialog {
				id: fileDialog
				onAccepted: CState.guildsStore.setPicture(routerInstance.guildHomeserver, routerInstance.guildID, this.fileUrl)
			}

			QQC2.Button {
				text: qsTr("Change Avatar")
				enabled: tryit(() => canUpdateInfo.data.has, false)
				onClicked: fileDialog.open()
			}
		}
	}
}

