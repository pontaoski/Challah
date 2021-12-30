import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.14 as Kirigami
import com.github.HarmonyDevelopment.Challah 1.0

ColumnLayout {
	RelationalListener {
		id: embedData
		model: timelineView.model.store
		key: del.messageID
		shape: QtObject {
			required property var contentEmbed
		}
	}

	QQC2.Label {
		text: del.resolvedName
		color: Kirigami.NameUtils.colorsFromString(text)

		visible: del.separateFromPrevious && !(del.isOwnMessage && Kirigami.Settings.isMobile)

		wrapMode: Text.Wrap
		maximumLineCount: 2
		elide: Text.ElideRight

		Layout.fillWidth: true
		Layout.leftMargin: Kirigami.Units.largeSpacing
	}

	ReplyBlock { Layout.leftMargin: Kirigami.Units.largeSpacing }

	Repeater {
		model: embedData.data.contentEmbed.embeds
		delegate: Embed {
			embedData: modelData

			Layout.maximumWidth: del.recommendedSize
			Layout.leftMargin: Kirigami.Units.largeSpacing
		}
	}
}
