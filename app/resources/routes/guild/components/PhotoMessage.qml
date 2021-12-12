import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.14 as Kirigami
import com.github.HarmonyDevelopment.Challah 1.0

QQC2.Control {
	id: photoMessageRoot

	topPadding: 0
	bottomPadding: 0
	leftPadding: 0+tailSize
	rightPadding: 0

	readonly property int tailSize: Kirigami.Units.largeSpacing

	background: MessageBackground {
		id: _background
		tailSize: photoMessageRoot.tailSize
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
		ReplyBlock {}
		Repeater {
			model: photosData.data.contentPhotos.photos

			delegate: ColumnLayout {
				id: del

				required property var modelData

				Image {
					source: CState.mediaURL(del.modelData.hmc)

					sourceHeight: del.modelData.height
					sourceWidth: del.modelData.width

					layer.enabled: true
					layer.effect: OpacityMask {
						maskSource: Rectangle {
							color: "red"
							radius: 4
							width: image.width
							height: image.height
						}
					}
				}

				TextEdit {
					id: textEdit
					text: del.modelData.caption.text
					Component.onCompleted: UIUtils.formatDocument(CState, textEdit.textDocument, textEdit, del.modelData.caption)
					Connections {
						target: del
						function onModelDataChanged() {
							UIUtils.formatDocument(CState, textEdit.textDocument, textEdit, del.modelData.caption)
						}
					}
					visible: text !== ""

					readOnly: true
					selectByMouse: !Kirigami.Settings.isMobile
					wrapMode: Text.Wrap

					color: Kirigami.Theme.textColor
					selectedTextColor: Kirigami.Theme.highlightedTextColor
					selectionColor: Kirigami.Theme.highlightColor

					onLinkActivated: (mu) => {
						Qt.openUrlExternally(mu)
					}

					HoverHandler {
						acceptedButtons: Qt.NoButton
						cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
					}

					Layout.fillWidth: true
				}
			}
		}
	}

	RelationalListener {
		id: photosData
		model: timelineView.model.store
		key: del.messageID
		shape: QtObject {
			required property var contentPhotos
		}
	}

	Layout.maximumWidth: del.recommendedSize
}
