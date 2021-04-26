// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Layouts 1.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Challah 1.0

Kirigami.PageRoute {
	name: "loading"
	cache: false

	Kirigami.Page {
		globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None

		QQC2.BusyIndicator {
			anchors.centerIn: parent
		}
	}
}


