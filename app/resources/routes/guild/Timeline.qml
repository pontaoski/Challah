// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Challah 1.0

import "components" as Components

Kirigami.PageRoute {
name: "Guild/Timeline"
cache: true

Kirigami.Page {
	id: page

	padding: 0
	globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None
	Kirigami.Theme.colorSet: Kirigami.Theme.View

	OverlappingPanels {
		anchors.fill: parent

		leftPanel: LeftHandDrawer { }
		centerPanel: CenterHandDrawer { }
		rightPanel: RightHandDrawer { }
	}
}

}
