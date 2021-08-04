// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtGraphicalEffects 1.15
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.14 as Kirigami

Item {
	id: backgroundRoot

	required property int tailSize
	property alias timestamp: _row
	property alias icon: _icon
	property alias timestampText: timestamp.text
	readonly property string textPadding: " ".repeat(Math.ceil(_row.width / dummy.implicitWidth)) + "⠀"

	clip: true

	Item {
		id: tailBase
		clip: true
		visible: false

		anchors {
			top: parent.top
			bottom: parent.bottom
			left: parent.left
			leftMargin: -backgroundRoot.tailSize*2
			rightMargin: -backgroundRoot.tailSize
			right: mainBG.left
		}
		Rectangle {
			color: Kirigami.Theme.backgroundColor

			anchors.fill: parent
			anchors.topMargin: 4
			anchors.rightMargin: -backgroundRoot.tailSize
		}
	}
	Item {
		id: tailMask
		clip: true
		visible: false

		anchors {
			top: parent.top
			bottom: parent.bottom
			left: parent.left
			leftMargin: -backgroundRoot.tailSize*2
			rightMargin: -backgroundRoot.tailSize
			right: mainBG.left
		}
		Kirigami.ShadowedRectangle {
			anchors.fill: parent
			anchors.rightMargin: backgroundRoot.tailSize

			width: backgroundRoot.tailSize*3
			color: "black"

			corners {
				topLeftRadius: 0
				topRightRadius: 0
				bottomRightRadius: backgroundRoot.tailSize*10
				bottomLeftRadius: 0
			}
		}
	}
	OpacityMask {
		anchors.fill: tailBase
		source: tailBase
		maskSource: tailMask
		invert: true
		visible: del.showAvatar
	}
	Kirigami.ShadowedRectangle {
		id: mainBG
		corners {
			topLeftRadius: 4
			topRightRadius: 4
			bottomRightRadius: 4
			bottomLeftRadius: 4
		}
		color: Kirigami.Theme.backgroundColor
		anchors.fill: parent
		anchors.leftMargin: backgroundRoot.tailSize
	}
	Row {
		id: _row
		spacing: 2

		LayoutMirroring.enabled: Qt.application.layoutDirection == Qt.RightToLeft
		anchors {
			bottom: parent.bottom
			right: mainBG.right
			margins: Kirigami.Units.smallSpacing
			rightMargin: Kirigami.Units.largeSpacing+2
		}
		Kirigami.Icon {
			id: _icon

			width: 16
			height: 16
			opacity: 0.5

			visible: false

			anchors {
				bottom: timestamp.bottom
			}
		}
		QQC2.Label {
			id: timestamp
			opacity: 0.5

			font.pointSize: -1
			font.pixelSize: Kirigami.Units.gridUnit * (2/3)
		}
	}
	QQC2.Label {
		id: dummy
		text: " "
	}
	layer.enabled: true
	layer.effect: DropShadow {
		cached: true
		horizontalOffset: 0
		verticalOffset: 1
		radius: 2.0
		samples: 17
		color: "#30000000"
	}
}
