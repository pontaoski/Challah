// SPDX-FileCopyrightText: 2022 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.14 as Kirigami
import org.kde.kitemmodels 1.0
import com.github.HarmonyDevelopment.Challah 1.0

Kirigami.ScrollablePage {
    topPadding: 0
    leftPadding: 0
    bottomPadding: 0
    rightPadding: 0

    titleDelegate: RowLayout {
        QQC2.ToolButton {
            icon.name: "arrow-left"
            onClicked: colView.layers.pop()
        }
        Kirigami.Heading {
            text: qsTr("Guild Settings")
            level: 4
        }
        Item { Layout.fillWidth: true }
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
        spacing: 0

        Kirigami.Avatar {
            Layout.preferredHeight: Kirigami.Units.iconSizes.huge
            Layout.preferredWidth: Kirigami.Units.iconSizes.huge
            Layout.margins: Kirigami.Units.gridUnit

            Layout.alignment: Qt.AlignHCenter

            name: tryit(() => guildData.data.name, "")
            source: tryit(() => guildData.data.picture, "")
        }

        Kirigami.Heading {
            text: tryit(() => guildData.data.name, "")

            horizontalAlignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.gridUnit
        }

        RightArrowListItem {
            text: qsTr("Change Info")
            onClicked: colView.layers.push(Qt.resolvedUrl("GuildInfo.qml"))

            Layout.fillWidth: true
        }

        RightArrowListItem {
            text: qsTr("Manage Invites")
            onClicked: colView.layers.push(Qt.resolvedUrl("Invites.qml"))

            Layout.fillWidth: true
        }

        RightArrowListItem {
            text: qsTr("Manage Channels")
            onClicked: colView.layers.push(Qt.resolvedUrl("Channels.qml"))

            Layout.fillWidth: true
        }

        RightArrowListItem {
            text: qsTr("Manage Roles")
            onClicked: colView.layers.push(Qt.resolvedUrl("Roles.qml"))

            Layout.fillWidth: true
        }
    }
}