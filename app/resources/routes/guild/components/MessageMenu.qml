import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.14 as Kirigami
import com.github.HarmonyDevelopment.Challah 1.0

QQC2.Menu {
	id: messageMenu

	// QQC2.MenuItem {
	// 	text: qsTr("Edit")
	// 	enabled: messagesRoute.model.permissions.canSendAndEdit && messagesRoute.model.userID() == authorID
	// 	onTriggered: {
	// 		messageBlock.edit = true
	// 	}
	// }
	QQC2.MenuItem {
		text: qsTr("Delete")
		enabled: tryit(() => canDeletePermissions.data.has || del.isOwnMessage, false)
		onTriggered: {
			timelineView.model.deleteMessage(del.messageID)
		}
	}
	QQC2.MenuItem {
		text: qsTr("Reply")
		enabled: tryit(() => canSendPermissions.data.has, false)
		onTriggered: {
			page.interactionID = del.messageID
			page.interactionKind = "reply"
		}
	}
	// QQC2.MenuItem {
	// 	text: qsTr("Select")
	// 	onTriggered: {
	// 		messagesSelectionModel.select(modelIndex, ItemSelectionModel.Select)
	// 	}
	// }
}
