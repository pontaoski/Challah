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
	id: settingsPage
	title: qsTr("Settings")
	padding: 0
	Kirigami.Theme.colorSet: Kirigami.Theme.View

	property var invitesModel: null
	property var rolesModel: null

	Kirigami.SwipeNavigator {
		anchors.fill: parent

		Invites { visible: settingsPage.invitesModel != null }
		Roles { visible: settingsPage.rolesModel != null }
	}
}
