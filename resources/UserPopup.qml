// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Layouts 1.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

QQC2.Popup {
	id: userPopupRoot

	background: Kirigami.Card {}

	property string name: ""
	property string source: ""

	width: Kirigami.Units.gridUnit * 10
	leftPadding: Kirigami.Units.gridUnit*2
	rightPadding: Kirigami.Units.gridUnit*2

	enter: Transition {
		ParallelAnimation {
			NumberAnimation { property: "x"; from: -20 }
			NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
		}
	}
	exit: Transition {
		ParallelAnimation {
			NumberAnimation { property: "x"; to: from+20 }
			NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
		}
	}

	contentItem: ColumnLayout {
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
	}
}
