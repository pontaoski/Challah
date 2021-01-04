// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Layouts 1.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.OverlaySheet {
	id: userPopupRoot

	background: Kirigami.Card {}

	property string name: ""
	property string source: ""

	leftPadding: Kirigami.Units.gridUnit*2
	rightPadding: Kirigami.Units.gridUnit*2

	contentItem: ColumnLayout {
		Layout.maximumWidth: Kirigami.Units.gridUnit * 20

		Kirigami.Avatar {
			name: userPopupRoot.name
			source: userPopupRoot.source

			Layout.alignment: Qt.AlignHCenter
		}
		Kirigami.Heading {
			text: userPopupRoot.name
			level: 2

			horizontalAlignment: Text.AlignHCenter
			Layout.fillWidth: true
		}

		Kirigami.Heading {
			text: qsTr("Roles")
			level: 4
		}

		Kirigami.Heading {
			text: qsTr("Manage User")
			level: 4
		}

		Kirigami.BasicListItem {
			text: qsTr("Kick User")

			topPadding: Kirigami.Units.largeSpacing
			bottomPadding: Kirigami.Units.largeSpacing

			Layout.fillWidth: true
		}

		Kirigami.BasicListItem {
			text: qsTr("Ban User")

			topPadding: Kirigami.Units.largeSpacing
			bottomPadding: Kirigami.Units.largeSpacing

			Layout.fillWidth: true
		}
	}
}
