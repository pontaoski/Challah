import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.14 as Kirigami
import com.github.HarmonyDevelopment.Challah 1.0

import "qrc:/components" as GlobalComponents

QQC2.Control {
	id: del

	required property string previousMessageID
	required property string messageID
	required property string nextMessageID

	readonly property string resolvedAvatar: messageData.data.overrideAvatar || userData.data.avatarURL
	readonly property string resolvedName: messageData.data.overrideName || userData.data.name

	readonly property int recommendedSize: Math.max(del.width / 3, Kirigami.Units.gridUnit * 15) // (rootRow.wideMode ? Math.max(del.width / 3, Kirigami.Units.gridUnit * 15) : (del.width * 0.8))

	readonly property bool isOwnMessage: messageData.data.author === CState.ownID
	readonly property bool showAvatar: del.nextMessageID == "" || (nextData.data.author != messageData.data.author) || (nextData.data.overrideAvatar != messageData.data.overrideAvatar) || (nextData.data.overrideName != messageData.data.overrideName) // && (!(Kirigami.Settings.isMobile && isOwnMessage))
	readonly property bool separateFromPrevious: del.previousMessageID == "" || (previousData.data.author != messageData.data.author) || (previousData.data.overrideAvatar != messageData.data.overrideAvatar) || (previousData.data.overrideName != messageData.data.overrideName)
	// readonly property bool canDeleteMessage: isOwnMessage

	// topPadding: del.separateFromPrevious ? Kirigami.Units.largeSpacing : Kirigami.Units.smallSpacing
	bottomPadding: 0

	Accessible.role: Accessible.ListItem
	Accessible.name: "mu"

	// Kirigami.Theme.backgroundColor: {
	// 	if (isOwnMessage)
	// 		return Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.focusColor, 0.1)

	// 	if (Kirigami.ColorUtils.brightnessForColor(messagesViewRoot.Kirigami.Theme.backgroundColor) == Kirigami.ColorUtils.Light)
	// 		return Qt.darker(messagesViewRoot.Kirigami.Theme.backgroundColor, 1.1)
	// 	else
	// 		return Qt.lighter(messagesViewRoot.Kirigami.Theme.backgroundColor, 1.3)
	// }

	Kirigami.Theme.colorSet: {
		return Kirigami.Theme.Button
		// if (Array.from(messagesSelectionModel.selectedIndexes).includes(modelIndex)) {
		//     return Kirigami.Theme.Selection
		// }
		// return messagesRoute.model.userID() == authorID ? Kirigami.Theme.Button : Kirigami.Theme.Window
	}
	Kirigami.Theme.inherit: false

	MessageMenu {
		id: messageMenu
	}

	TapHandler {
		acceptedButtons: Kirigami.Settings.isMobile ? Qt.LeftButton : Qt.RightButton
		onTapped: messageMenu.popup()
	}

	contentItem: RowLayout {
		Kirigami.Avatar {
			name: del.resolvedName
			source: del.resolvedAvatar

			implicitWidth: Kirigami.Units.gridUnit*2
			implicitHeight: Kirigami.Units.gridUnit*2

			// visible yoinks from layout, which isn't what we want.
			opacity: del.showAvatar ? 1 : 0

			// visible: !settings.thinMode

			Layout.alignment: Qt.AlignBottom
		}

		// Item {
		// 	Layout.fillWidth: (del.isOwnMessage && Kirigami.Settings.isMobile)
		// }

		Loader {
			id: loader

			source: {
				switch (messageData.data.contentType) {
				case "textMessage":
					return Qt.resolvedUrl("TextMessage.qml")
				case "filesMessage":
					return Qt.resolvedUrl("FileMessage.qml")
				case "embedMessage":
					return Qt.resolvedUrl("EmbedMessage.qml")
				}
				return Qt.resolvedUrl("Unsupported.qml")
			}

			Binding { target: loader; property: "Layout.fillWidth"; when: loader.item !== null; value: ((loader.item || {}).Layout || {}).fillWidth }
			Binding { target: loader; property: "Layout.maximumWidth"; when: loader.item !== null; value: ((loader.item || {}).Layout || {}).maximumWidth }
			Binding { target: loader; property: "Layout.topMargin"; when: loader.item !== null; value: ((loader.item || {}).Layout || {}).topMargin }
			Binding { target: loader; property: "Layout.preferredHeight"; when: loader.item !== null; value: ((loader.item || {}).Layout || {}).preferredHeight }
			Binding { target: loader; property: "Layout.leftMargin"; when: loader.item !== null; value: ((loader.item || {}).Layout || {}).leftMargin }
		}

		Item {
			Layout.fillWidth: true
		}
	}

	width: parent && parent.width > 0 ? parent.width : implicitWidth
	Layout.fillWidth: true

	RelationalListener {
		id: messageData

		model: timelineView.model.store
		key: del.messageID
		shape: QtObject {
			required property string author
			required property string contentType
			required property string timestamp
			required property string overrideAvatar
			required property string overrideName
		}
	}
	RelationalListener {
		id: previousData

		model: timelineView.model.store
		enabled: del.previousMessageID != "0"
		key: del.previousMessageID
		shape: QtObject {
			required property string author
			required property string overrideAvatar
			required property string overrideName
		}
	}
	RelationalListener {
		id: nextData

		model: timelineView.model.store
		enabled: del.nextMessageID != "0"
		key: del.nextMessageID
		shape: QtObject {
			required property string author
			required property string overrideAvatar
			required property string overrideName
		}
	}
	RelationalListener {
		id: userData

		model: CState.membersStore
		key: tryit(() => [page.homeserver, messageData.data.author], ["", ""])
		shape: QtObject {
			required property string name
			required property string avatarURL
		}
	}
}
