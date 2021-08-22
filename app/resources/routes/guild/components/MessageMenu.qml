import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.14 as Kirigami
import com.github.HarmonyDevelopment.Challah 1.0

QQC2.Menu {
	id: messageMenu

	RelationalListener {
		id: canDeletePermissions

		model: CState.ownPermissionsStore
		key: [routerInstance.params.homeserver, routerInstance.params.guildID, routerInstance.params.channelID, "messages.manage.delete"]
		shape: QtObject {
			required property bool has
		}
	}

	// QQC2.MenuItem {
	// 	text: qsTr("Edit")
	// 	enabled: messagesRoute.model.permissions.canSendAndEdit && messagesRoute.model.userID() == authorID
	// 	onTriggered: {
	// 		messageBlock.edit = true
	// 	}
	// }
	QQC2.MenuItem {
		text: qsTr("Delete")
		enabled: canDeletePermissions.data.has
		onTriggered: {
			timelineView.model.deleteMessage(del.messageID)
		}
	}
	// QQC2.MenuItem {
	// 	text: qsTr("Reply")
	// 	enabled: messagesRoute.model.permissions.canSendAndEdit
	// 	onTriggered: {
	// 		composeBar.replies.replyingToID = messageID
	// 		composeBar.replies.replyingToAuthor = authorName
	// 		composeBar.replies.replyingToContent = content
	// 	}
	// }
	// QQC2.MenuItem {
	// 	text: qsTr("Select")
	// 	onTriggered: {
	// 		messagesSelectionModel.select(modelIndex, ItemSelectionModel.Select)
	// 	}
	// }
}
