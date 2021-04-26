// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Layouts 1.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Challah 1.0

Kirigami.PageRoute {
name: "Login/Stepper"
cache: false

Kirigami.Page {
	id: thePage
	title: qsTr("Login")

	property Item current: Item {}
	Kirigami.Theme.colorSet: Kirigami.Theme.View

	Connections {
		target: CState
		function onPlaceItem(item) {
			thePage.current.visible = false
			thePage.current = item
			item.parent = thePage
			item.anchors.centerIn = thePage
		}
	}
}
}
