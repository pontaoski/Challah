// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2

Kirigami.Page {
	id: settingsPage
	title: qsTr("Settings")
	padding: 0
	Kirigami.Theme.colorSet: Kirigami.Theme.View

	globalToolBarStyle: 0

	Kirigami.SwipeNavigator {
		anchors.fill: parent

		header: QQC2.Button {
			icon.name: "go-previous"
			onClicked: colView.layers.pop()
		}

		GuildData {}
		Invites {}
		Roles {}
		Channels {}
	}
}
