// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.14 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import QtGraphicalEffects 1.0
import com.github.HarmonyDevelopment.Staccato 1.0
import QtQml.Models 2.15

QQC2.Control {
	padding: 0
	topPadding: 0
	leftPadding: 0
	rightPadding: 0
	bottomPadding: 0

	background: MouseArea {
		id: backgroundMA
		acceptedButtons: Qt.LeftButton | Qt.RightButton
		onClicked: {
			if (messagesSelectionModel.selection.length > 0) {
				if (mouse.button === Qt.RightButton) {
					selectionMenu.open()
				} else {
					messagesSelectionModel.select(modelIndex, ItemSelectionModel.Toggle)
				}
			} else if (mouse.button === Qt.RightButton) {
				messageMenu.open()
			}
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
			ResponsiveMenuItem {
				text: qsTr("Select")
				onTriggered: {
					messagesSelectionModel.select(modelIndex, ItemSelectionModel.Select)
				}
			}
		}
		ResponsiveMenu {
			id: selectionMenu
			ResponsiveMenuItem {
				text: qsTr("Copy Selected Messages")
				onTriggered: {
					let field = Qt.createQmlObject("import QtQuick 2.10; TextEdit { visible: false }", root, "<embedded>")
					field.text = Array.from(messagesSelectionModel.selectedIndexes).map(function(item) {
						return `${messagesSelectionModel.model.data(item, 259)} [${messagesSelectionModel.model.data(item, 263).toLocaleString()}]\n${messagesSelectionModel.model.data(item, 256)}`
					}).join("\n\n")
					field.selectAll()
					field.copy()
				}
			}
			ResponsiveMenuItem {
				text: Array.from(messagesSelectionModel.selectedIndexes).includes(modelIndex) ? qsTr("Unselect") : qsTr("Select")
				onTriggered: {
					messagesSelectionModel.select(modelIndex, ItemSelectionModel.Toggle)
				}
			}
		}
	}

	contentItem: ColumnLayout {
		id: messageDelegate
		property string modelMessageID: messageID

		Kirigami.Theme.colorSet: {
			if (Array.from(messagesSelectionModel.selectedIndexes).includes(modelIndex)) {
				return Kirigami.Theme.Selection
			}
			return messagesRoute.model.userID() == authorID ? Kirigami.Theme.Button : Kirigami.Theme.Window
		}
		spacing: 0

		Item {
			implicitHeight: yote.implicitHeight + Kirigami.Units.gridUnit
			visible: !!quirks["dateHeader"]

			Kirigami.Heading {
				id: yote
				level: 4
				text: quirks["dateHeader"] || ""
				anchors.centerIn: parent
				padding: Kirigami.Units.smallSpacing
				leftPadding: Kirigami.Units.largeSpacing
				rightPadding: Kirigami.Units.largeSpacing

				background: Rectangle {
					radius: height

					Kirigami.Theme.colorSet: Kirigami.Theme.Window
					color: Kirigami.Theme.backgroundColor
				}
			}

			Layout.preferredWidth: (applicationWindow().wideScreen ? Math.max(messagesView.width / 3, Kirigami.Units.gridUnit * 15) : (messagesView.width * 0.9)) - Layout.leftMargin
			Layout.leftMargin: Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing
		}

		Item { height: !!quirks["previousAuthorDifferent"] ? 6 : 2 }

		QQC2.Control {
			id: messageBlock
			property bool edit: false
			visible: content != ""

			padding: Kirigami.Units.largeSpacing * 2

			Layout.minimumWidth: Kirigami.Units.gridUnit * 3
			Layout.maximumWidth: (applicationWindow().wideScreen ? Math.max(messagesView.width / 3, Kirigami.Units.gridUnit * 15) : (messagesView.width * 0.9)) - Layout.leftMargin
			Layout.preferredWidth: theThing.data.length > 0 ? Layout.maximumWidth : -1
			Layout.leftMargin: Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing
			Kirigami.Theme.colorSet: messagesRoute.model.userID() == authorID ? Kirigami.Theme.Button : Kirigami.Theme.Window

			implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth + leftPadding + rightPadding)
			implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, implicitContentHeight + topPadding + bottomPadding)

			background: Rectangle {
				radius: 4
				color: Kirigami.Theme.backgroundColor

				QQC2.Label {
					text: `${("0"+date.getHours()).slice(-2)}:${("0"+date.getMinutes()).slice(-2)}`
					opacity: 0.5
					visible: !messageBlock.edit

					font.pixelSize: Kirigami.Units.gridUnit * (1/2)
					font.pointSize: -1
					anchors {
						bottom: parent.bottom
						right: parent.right
						margins: Kirigami.Units.smallSpacing
					}
				}
			}
			contentItem: ColumnLayout {
				QQC2.Label {
					visible: !!quirks["previousAuthorDifferent"] || !!quirks["dateHeader"]
					text: authorName
					color: Kirigami.NameUtils.colorsFromString(authorName)

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
					TextEdit {
						text: readOnly ? content + "     ⠀" : content
						textFormat: TextEdit.MarkdownText
						readOnly: !messageBlock.edit

						font.pixelSize: Kirigami.Units.gridUnit * (3/4)
						font.pointSize: -1
						wrapMode: Text.Wrap

						selectByMouse: true

						color: Kirigami.Theme.textColor
						selectedTextColor: Kirigami.Theme.highlightedTextColor
						selectionColor: Kirigami.Theme.highlightColor

						Layout.alignment: Qt.AlignBottom
						Layout.maximumWidth: messageBlock.Layout.maximumWidth * 0.9

						Keys.onEscapePressed: {
							messageBlock.edit = false
							text = Qt.binding(function() { return readOnly ? content + "     ⠀" : content })
						}
						Keys.onReturnPressed: {
							messageBlock.edit = false
							messagesRoute.model.editMessage(messageID, text)
							text = Qt.binding(function() { return readOnly ? content + "     ⠀" : content })
						}
					}
				}
				Repeater {
					id: linkPreviewRepeater
					model: theThing.data
					delegate: QQC2.Control {
						id: theDelegate
						Layout.preferredWidth: ((applicationWindow().wideScreen ? Math.max(messagesView.width / 3, Kirigami.Units.gridUnit * 15) : (messagesView.width * 0.9))) - Kirigami.Units.largeSpacing * 8

						background: Rectangle {
							color: Kirigami.Theme.backgroundColor

							Image {
								anchors.fill: parent
								source: modelData["preview_image"]
								fillMode: Image.PreserveAspectCrop

								layer.enabled: true
								layer.effect: HueSaturation {
									cached: true
									saturation: 0.9

									layer.enabled: true
									layer.effect: GaussianBlur {
										cached: true

										radius: 32
										deviation: 12
										samples: 63

										transparentBorder: false
									}
								}
							}
							Rectangle {
								anchors.fill: parent
								color: Kirigami.Theme.backgroundColor
								opacity: 0.9
							}
							Rectangle {
								anchors {
									left: parent.left
									top: parent.top
									bottom: parent.bottom
									margins: Kirigami.Units.smallSpacing
								}
								width: 2
								color: Kirigami.Theme.highlightColor
							}
						}
						contentItem: ColumnLayout {
							spacing: 2

							QQC2.Label {
								visible: text != ""
								text: modelData["site_title"]
								color: Kirigami.Theme.highlightColor
								wrapMode: Text.Wrap

								Layout.fillWidth: true
								Layout.leftMargin: Kirigami.Units.smallSpacing * 2
							}
							QQC2.Label {
								text: modelData["page_title"]
								wrapMode: Text.Wrap

								Layout.fillWidth: true
								Layout.leftMargin: Kirigami.Units.smallSpacing * 2
							}
							QQC2.Label {
								text: modelData["description"]
								wrapMode: Text.Wrap

								Layout.fillWidth: true
								Layout.leftMargin: Kirigami.Units.smallSpacing * 2
							}
							QQC2.Button {
								text: qsTr("Instant View")
								visible: modelData["instant_view_ok"]
								flat: true

								onClicked: {
									let item = root.pageStack.layers.push(Qt.resolvedUrl("InstantView.qml"))
									messagesRoute.model.parentModel.grabInstantView(modelData["from_url"], function(data) {
										item.title = modelData["page_title"]
										item.theData = data
									})
								}

								Layout.fillWidth: true
								Layout.leftMargin: Kirigami.Units.smallSpacing * 2
							}
						}
					}
				}
				Item { visible: theThing.data.length > 0; implicitHeight: Kirigami.Units.largeSpacing }
			}
		}
		QtObject {
			id: theThing
			property string text: content
			property var data: []
			onTextChanged: {
				theThing.data = []
				var urlRegex =/(\b(https?):\/\/[-A-Z0-9+&@#\/%?=~_|!:,.;]*[-A-Z0-9+&@#\/%=~_|])/ig
				var matches = (content.match(urlRegex) || [])
				if (matches.length > 0) {
					messagesRoute.model.parentModel.checkCanInstantView(matches, function(data) {
						theThing.data = data
					})
				}
			}
		}
		Repeater {
			model: Array.from(attachments).filter(item => item["type"].startsWith("image"))
			delegate: Image {
				source: HState.transformHMCURL(modelData["id"], messagesRoute.model.homeserver)
				fillMode: Image.PreserveAspectCrop
				smooth: true
				mipmap: true

				Component {
					id: imagePopupComponent

					ImagePopup {
						source: HState.transformHMCURL(modelData["id"], messagesRoute.model.homeserver)
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
