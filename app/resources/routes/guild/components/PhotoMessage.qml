import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.14 as Kirigami
import QtGraphicalEffects 1.15
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
			maximumLineCount: 2
			elide: Text.ElideRight

			Layout.fillWidth: true
		}
		ReplyBlock {}
		Repeater {
			model: photosData.data.contentPhotos.photos

			delegate: ColumnLayout {
				id: del

				required property var modelData
				readonly property string text: tryit(() => del.modelData.caption.text, "")
				readonly property var contData: tryit(() => del.modelData.caption, {})

				Image {
					id: image

					source: CState.mediaURL(del.modelData.hmc, routerInstance.guildHomeserver)

					Rectangle {
						color: Kirigami.Theme.backgroundColor
						anchors.fill: image
						visible: image.status !== Image.Ready

						Loader {
							anchors.fill: parent
							active: image.status !== Image.Ready
							sourceComponent: Image {
								anchors.fill: parent
								source: `data:img/jpeg;base64,` + del.modelData.minithumbnail.data

								layer.enabled: true
								layer.effect: FastBlur {
									cached: true
									radius: 32
								}
							}
						}
					}

					readonly property real ratio: width / sourceSize.width
					Layout.preferredHeight: sourceSize.height * image.ratio
					Layout.preferredWidth: sourceSize.width
					Layout.fillWidth: true
					smooth: true
					mipmap: true

					sourceSize.height: del.modelData.height
					sourceSize.width: del.modelData.width

					layer.enabled: true
					layer.effect: OpacityMask {
						maskSource: Rectangle {
							color: "red"
							radius: 4
							width: image.width
							height: image.height
						}
					}

					QQC2.Label {
						text: messageData.data.timestamp

						Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
						Kirigami.Theme.inherit: false

						font.pointSize: -1
						font.pixelSize: Kirigami.Units.gridUnit * (2/3)

						padding: Kirigami.Units.smallSpacing
						leftPadding: Math.floor(Kirigami.Units.smallSpacing*(3/2))
						rightPadding: Math.floor(Kirigami.Units.smallSpacing*(3/2))

						visible: !textEdit.visible

						anchors {
							bottom: parent.bottom
							right: parent.right
							margins: Kirigami.Units.largeSpacing
						}
						background: Rectangle {
							color: Kirigami.Theme.backgroundColor
							opacity: 0.7
							radius: 3
						}
					}
				}

				TextEdit {
					id: textEdit
					text: del.text
					Component.onCompleted: UIUtils.formatDocument(CState, textEdit.textDocument, textEdit, del.contData)
					Connections {
						target: del
						function onModelDataChanged() {
							UIUtils.formatDocument(CState, textEdit.textDocument, textEdit, del.contData)
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
