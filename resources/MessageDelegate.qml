// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

QQC2.Control {
	padding: 0
	topPadding: 0
	leftPadding: 0
	rightPadding: 0
	bottomPadding: 0

	background: MouseArea {
		acceptedButtons: Qt.RightButton
		onClicked: {
			messageMenu.open()
		}

		ResponsiveMenu {
			id: messageMenu
			ResponsiveMenuItem {
				text: qsTr("Edit")
				enabled: messagesRoute.model.permissions.canSendAndEdit && messagesRoute.model.userID() == authorID
				onTriggered: {
					messageBlock.edit = true
				}
			}
			ResponsiveMenuItem {
				text: qsTr("Delete")
				enabled: messagesRoute.model.permissions.canDeleteOthers || messagesRoute.model.userID() == authorID
				onTriggered: {
					messagesRoute.model.deleteMessage(messageID)
				}
			}
			ResponsiveMenuItem {
				text: qsTr("Reply")
				enabled: messagesRoute.model.permissions.canSendAndEdit
				onTriggered: {
					composeBar.replies.replyingToID = messageID
					composeBar.replies.replyingToAuthor = authorName
					composeBar.replies.replyingToContent = content
				}
			}
		}
	}

	contentItem: ColumnLayout {
		id: messageDelegate
		property string modelMessageID: messageID

		Kirigami.Theme.colorSet: messagesRoute.model.userID() == authorID ? Kirigami.Theme.Button : Kirigami.Theme.Window

		QQC2.Control {
			id: messageBlock
			property bool edit: false
			visible: content != ""

			padding: Kirigami.Units.largeSpacing * 2

			Layout.minimumWidth: Kirigami.Units.gridUnit * 3
			Layout.maximumWidth: (applicationWindow().wideScreen ? Math.max(messagesView.width / 3, Kirigami.Units.gridUnit * 15) : (messagesView.width * 0.9)) - Layout.leftMargin
			Layout.leftMargin: Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing
			Kirigami.Theme.colorSet: messagesRoute.model.userID() == authorID ? Kirigami.Theme.Button : Kirigami.Theme.Window

			implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth + leftPadding + rightPadding)
			implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, implicitContentHeight + topPadding + bottomPadding)

			background: Rectangle {
				radius: 4
				color: Kirigami.Theme.backgroundColor
			}
			contentItem: ColumnLayout {
				QQC2.Label {
					visible: shouldShowAuthorInfo
					text: authorName

					font.pixelSize: Kirigami.Units.gridUnit * (4/5)
					font.pointSize: -1
					wrapMode: Text.Wrap

					Layout.alignment: Qt.AlignTop
				}
				QQC2.Control {
					visible: replyToID !== undefined
					padding: 0

					background: Item {
						Rectangle {
							anchors {
								left: parent.left
								top: parent.top
								bottom: parent.bottom
							}
							width: 2
							color: Kirigami.Theme.highlightColor
						}
					}
					contentItem: ColumnLayout {
						property var peeked: replyToID !== undefined ? messagesView.model.peekMessage(replyToID) || {} : {}
						spacing: 1
						QQC2.Label {
							text: parent.peeked["authorName"] || ""
							elide: Text.ElideRight
							color: Kirigami.Theme.highlightColor
							textFormat: TextEdit.MarkdownText

							Layout.fillWidth: true
						}
						QQC2.Label {
							text: parent.peeked["content"] || ""
							elide: Text.ElideRight
							textFormat: TextEdit.MarkdownText

							Layout.fillWidth: true
						}
					}

					Layout.fillWidth: true
					Layout.maximumWidth: messageBlock.Layout.maximumWidth * 0.9
				}
				GridLayout {
					QQC2.Label {
						visible: !messageBlock.edit
						text: content
						textFormat: TextEdit.MarkdownText

						font.pixelSize: Kirigami.Units.gridUnit * (3/4)
						font.pointSize: -1
						wrapMode: Text.Wrap

						Layout.alignment: Qt.AlignBottom
						Layout.maximumWidth: messageBlock.Layout.maximumWidth * 0.9
					}
					QQC2.TextField {
						visible: messageBlock.edit
						text: content

						Keys.onEscapePressed: messageBlock.edit = false

						onAccepted: {
							messageBlock.edit = false
							messagesRoute.model.editMessage(messageID, text)
						}

						Layout.alignment: Qt.AlignBottom
					}
					Item { Layout.fillWidth: true }
					QQC2.Label {
						text: `${("0"+date.getHours()).slice(-2)}:${("0"+date.getMinutes()).slice(-2)}`
						opacity: 0.5

						font.pixelSize: Kirigami.Units.gridUnit * (1/2)
						font.pointSize: -1
						Layout.alignment: Qt.AlignBottom | Qt.AlignRight
					}
				}
			}
		}
		Repeater {
			model: attachments
			delegate: Image {
				source: HState.transformHMCURL(modelData)
				fillMode: Image.PreserveAspectCrop
				smooth: true
				mipmap: true

				Component {
					id: imagePopupComponent

					QQC2.Popup {
						id: imagePopup

						anchors.centerIn: QQC2.Overlay.overlay
						modal: true

						background: Item {}
						Image {
							id: popupImage
							source: HState.transformHMCURL(modelData)
							x: (parent.QQC2.Overlay.overlay.width / 2) - (this.implicitWidth / 2)
							y: (parent.QQC2.Overlay.overlay.height / 2) - (this.implicitHeight / 2)

							PinchArea {
								anchors.fill: parent
								pinch.target: popupImage
								pinch.minimumRotation: -360
								pinch.maximumRotation: 360
								pinch.minimumScale: 0.1
								pinch.maximumScale: 10
								pinch.dragAxis: Pinch.XAndYAxis
							}
							MouseArea {
								drag.target: parent
								anchors.fill: parent
								onWheel: {
									if (wheel.modifiers & Qt.ControlModifier) {
										popupImage.rotation += wheel.angleDelta.y / 120 * 5
										if (Math.abs(popupImage.rotation) < 4)
											popupImage.rotation = 0
									} else {
										popupImage.rotation += wheel.angleDelta.x / 120
										if (Math.abs(popupImage.rotation) < 0.6)
											popupImage.rotation = 0
										var scaleBefore = popupImage.scale
										popupImage.scale += popupImage.scale * wheel.angleDelta.y / 120 / 10
									}
								}
							}
						}

						width: QQC2.Overlay.overlay.width
						height: QQC2.Overlay.overlay.height

						onClosed: {
							this.destroy()
						}
					}
				}

				MouseArea {
					anchors.fill: parent
					onClicked: {
						let item = imagePopupComponent.createObject(messageDelegate)
						item.open()
					}
				}

				Layout.leftMargin: Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing
				Layout.preferredWidth: implicitWidth
				Layout.preferredHeight: implicitHeight
				Layout.maximumHeight: Layout.maximumWidth
				Layout.maximumWidth: (applicationWindow().wideScreen ? Math.max(messagesView.width / 3, Kirigami.Units.gridUnit * 15) : (messagesView.width * 0.9)) - Layout.leftMargin
			}
		}
		Repeater {
			model: actions
			delegate: MessageAction {
				messageID: messageDelegate.modelMessageID
				Layout.leftMargin: Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing
			}
		}
		Repeater {
			model: embeds
			delegate: Embed {
				Layout.leftMargin: Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing
				Layout.maximumWidth: (applicationWindow().wideScreen ? Math.max(messagesView.width / 3, Kirigami.Units.gridUnit * 15) : (messagesView.width * 0.9)) - Layout.leftMargin
			}
		}
	}
}
