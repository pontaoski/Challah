// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.14 as Kirigami
import com.github.HarmonyDevelopment.Challah 1.0

import "qrc:/components" as Components

QQC2.Control {
	visible: replyData.data.inReplyTo !== "0"
	topPadding: 0
	leftPadding: 6
	rightPadding: 0
	bottomPadding: 0

	RelationalListener {
		id: replyData

		model: timelineView.model.store
		key: del.messageID
		shape: QtObject {
			required property string inReplyTo
		}
	}
	Components.PlaintextMessage {
		id: repliedToData

		messagesStore: timelineView.model.store
		messageID: replyData.data.inReplyTo
		homeserver: page.homeserver
	}

	background: Item {
		Rectangle {
			anchors {
				left: parent.left
				top: parent.top
				bottom: parent.bottom
			}
			width: 2
			color: Kirigami.NameUtils.colorsFromString(repliedToData.authorName)
		}
	}
	contentItem: ColumnLayout {
		id: replyCol

		spacing: 1
		QQC2.Label {
			text: repliedToData.authorName
			elide: Text.ElideRight
			color: Kirigami.NameUtils.colorsFromString(repliedToData.authorName)

			Layout.fillWidth: true
		}
		QQC2.Label {
			text: repliedToData.onelinePlaintext
			elide: Text.ElideRight

			Layout.fillWidth: true
		}

		clip: true
	}

	Layout.fillWidth: true
	Layout.topMargin: 3
}
