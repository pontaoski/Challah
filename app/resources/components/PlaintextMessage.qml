// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.10
import com.github.HarmonyDevelopment.Challah 1.0

QtObject {
	id: plaintext

	required property var messagesStore
	required property string messageID
	required property string homeserver

	readonly property string authorName: tryit(() => universalData.data.overrideName || authorData.data.name, "")
	readonly property string onelinePlaintext: tryit(() => this.plaintext.split("\n")[0], "")
	readonly property string plaintext: {
		switch (tryit(() => [universalData.data.contentType, universalData.dummy][0], "")) {
		case "textMessage":
			return textData.data.contentText
		case "embedMessage":
			return qsTr("Embed")
		case "filesMessage":
			return qsTr("Files")
		default:
			return qsTr("Unsupported")
		}
	}

	property var universalData: RelationalListener {
		id: universalData

		property string dummy

		model: plaintext.messagesStore
		key: plaintext.messageID
		shape: QtObject {
			required property string author
			required property string contentType
			required property string timestamp
			required property string overrideAvatar
			required property string overrideName
		}
	}
	property var authorData: RelationalListener {
		id: authorData

		model: CState.membersStore
		key: tryit(() => [plaintext.homeserver, universalData.data.author], ["", ""])

		shape: QtObject {
			required property string name
		}
	}
	property var textData: RelationalListener {
		id: textData

		model: plaintext.messagesStore
		key: plaintext.messageID

		shape: QtObject {
			required property string contentText
		}
	}
}
