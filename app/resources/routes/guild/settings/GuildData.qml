// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
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

	title: qsTr("Guild Information")

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

				horizontalAlignment: Qt.AlignHCenter
				Layout.fillWidth: true
			}

			QQC2.Button {
				text: qsTr("Edit Name")
				enabled: tryit(() => canUpdateInfo.data.has, false)
				onClicked: textAsker.ask(qsTr("What do you want to call the guild?")).then((name) => CState.guildsStore.setName(routerInstance.guildHomeserver, routerInstance.guildID, name))

				Layout.alignment: Qt.AlignRight
			}

			Kirigami.Avatar {
				implicitWidth: 100
				implicitHeight: 100

				name: tryit(() => guildData.data.name, "")
				source: tryit(() => guildData.data.picture, "")

				Layout.alignment: Qt.AlignHCenter
			}

			Dialogs.FileDialog {
				id: fileDialog
				onAccepted: CState.guildsStore.setPicture(routerInstance.guildHomeserver, routerInstance.guildID, this.fileUrl)
			}

			QQC2.Button {
				text: qsTr("Upload New Picture")
				enabled: tryit(() => canUpdateInfo.data.has, false)
				onClicked: fileDialog.open()

				Layout.alignment: Qt.AlignRight
			}
		}
	}
}

