import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.14 as Kirigami
import com.github.HarmonyDevelopment.Challah 1.0

QQC2.Control {
	id: textMessageRoot

	topPadding: Kirigami.Units.largeSpacing
	bottomPadding: Kirigami.Units.largeSpacing
	leftPadding: Kirigami.Units.largeSpacing+tailSize
	rightPadding: Kirigami.Units.largeSpacing

	readonly property int tailSize: Kirigami.Units.largeSpacing

	background: MessageBackground {
		id: _background
		tailSize: textMessageRoot.tailSize
		timestampText: messageData.data.timestamp
	}

	contentItem: ColumnLayout {
		QQC2.Label {
			text: del.resolvedName
			color: Kirigami.NameUtils.colorsFromString(text)

			visible: del.separateFromPrevious && !(del.isOwnMessage && Kirigami.Settings.isMobile)

			wrapMode: Text.Wrap

			Layout.fillWidth: true
		}
		Repeater {
			model: fileData.data.contentAttachments.attachments

			delegate: RowLayout {
				id: del

				required property var modelData

				Kirigami.Icon {
					source: del.modelData.type.replace("/", "-")
					fallback: "unknown"
				}

				ColumnLayout {
					spacing: 0

					QQC2.Label {
						text: del.modelData.name
						wrapMode: Text.Wrap

						Layout.fillWidth: true
					}
					QQC2.Label {
						text: UIUtils.formattedSize(del.modelData.size) + _background.textPadding
						opacity: 0.7

						Layout.fillWidth: true
					}
				}
			}
		}
	}

	RelationalListener {
		id: fileData
		model: timelineView.model.store
		key: del.messageID
		shape: QtObject {
			required property var contentAttachments
		}
	}

	Layout.maximumWidth: del.recommendedSize
}
