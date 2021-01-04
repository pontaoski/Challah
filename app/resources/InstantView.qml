// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.ScrollablePage {
	id: instantViewPage
	title: qsTr("Loading...")

	property string theData: ""

	Kirigami.Theme.colorSet: Kirigami.Theme.View

	Item {
		height: editor.implicitHeight
		width: instantViewPage.width

		TextEdit {
			id: editor
			color: Kirigami.Theme.textColor
			selectedTextColor: Kirigami.Theme.highlightedTextColor
			selectionColor: Kirigami.Theme.highlightColor
			textFormat: TextEdit.MarkdownText
			text: instantViewPage.theData
			wrapMode: Text.Wrap
			width: Math.min(Kirigami.Units.gridUnit * 60, instantViewPage.width - Kirigami.Units.largeSpacing*4)
			anchors.horizontalCenter: parent.horizontalCenter
			readOnly: true
		}
	}
}

