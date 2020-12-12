// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.5
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.13 as QQC2
import QtGraphicalEffects 1.0
import com.github.HarmonyDevelopment.Staccato 1.0 as S

Item {
	id: avatarRoot

	enum ImageMode {
		AlwaysShowImage,
		AdaptiveImageOrInitals,
		AlwaysShowInitials
	}
	enum InitialsMode {
		UseInitials,
		UseIcon
	}

	/**
	* The given name of a user.
	*
	* The user's name will be used for generating initials and to provide the
	* accessible name for assistive technology.
	*/
	property string name

	/**
	* The source of the user's profile picture; an image.
	*/
	property alias source: avatarImage.source

	/**
	* How the button should represent the user when there is no image available.
	* * `UseInitials` - Use initials when the image is not available
	* * `UseIcon` - Use an icon of a user when the image is not available
	*/
	property int initialsMode: Avatar.InitialsMode.UseInitials

	/**
	* Whether the button should always show the image; show the image if one is
	* available and show initials when it is not; or always show initials.
	* * `AlwaysShowImage` - Always show the image; even if is not value
	* * `AdaptiveImageOrInitals` - Show the image if it is valid; or show initials if it is not
	* * `AlwaysShowInitials` - Always show initials
	*/
	property int imageMode: Avatar.ImageMode.AdaptiveImageOrInitals

	 /**
	 * Whether or not the image loaded from the provided source should be cached.
	 *
	 */
	 property alias cache: avatarImage.cache

	/**
	 * color: color
	 *
	 * The color to use for this avatar.
	 */
	property var color: undefined
	// We use a var instead of a color here to allow setting the colour
	// as undefined, which will result in a generated colour being used.

	/**
	 * actions.main: Kirigami.Action
	 * actions.secondary: Kirigami.Action
	 *
	 * Actions associated with this avatar.
	 *
	 * Note that the secondary action should only be used for shortcuts of actions
	 * elsewhere in your application's UI, and cannot be accessed on mobile platforms.
	 */
	property S.AvatarGroup actions: S.AvatarGroup {}

	readonly property QtObject border: QtObject {
		property int width: 2
		property color color: Qt.rgba(0,0,0,0.2)
	}

	implicitWidth: Kirigami.Units.iconSizes.large
	implicitHeight: Kirigami.Units.iconSizes.large

	Accessible.role: !!actions.main ? Accessible.Button : Accessible.Graphic
	Accessible.name: !!actions.main ? qsTr("%1 — %2").arg(name).arg(actions.main.text) : name
	Accessible.focusable: !!actions.main
	Accessible.onPressAction: {
		avatarRoot.actions.main.trigger()
	}

	QQC2.Control {
		anchors.fill: parent
		padding: 0
		topPadding: 0
		leftPadding: 0
		rightPadding: 0
		bottomPadding: 0

		background: Rectangle {
			radius: parent.width / 2
			gradient: Gradient {
				GradientStop { position: 0.0; color: __private.color }
				GradientStop { position: 1.0; color: Kirigami.ColorUtils.scaleColor(__private.color, {lightness: -50.0}) }
			}
			MouseArea {
				id: primaryMouse

				anchors.fill: parent
				hoverEnabled: true
				property bool mouseInCircle: {
					let x = avatarRoot.width / 2, y = avatarRoot.height / 2
					let xPrime = mouseX, yPrime = mouseY

					let distance = (x - xPrime) ** 2 + (y - yPrime) ** 2
					let radiusSquared = (Math.min(avatarRoot.width, avatarRoot.height) / 2) ** 2

					return distance < radiusSquared
				}

				onClicked: {
					if (mouseY > avatarRoot.height - secondaryRect.height && !!avatarRoot.actions.secondary) {
						avatarRoot.actions.secondary.trigger()
						return
					}
					if (!!avatarRoot.actions.main) {
						avatarRoot.actions.main.trigger()
					}
				}

				enabled: !!avatarRoot.actions.main || !!avatarRoot.actions.secondary
				cursorShape: containsMouse && mouseInCircle && enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

				states: [
					State {
						name: "secondaryRevealed"
						when: (!Kirigami.Settings.isMobile) && (!!avatarRoot.actions.secondary) && (primaryMouse.containsMouse && primaryMouse.mouseInCircle)
						PropertyChanges {
							target: secondaryRect
							visible: true
						}
					}
				]
			}
		}

		contentItem: Item {
			Kirigami.Heading {
				visible: avatarRoot.initialsMode == Avatar.InitialsMode.UseInitials &&
						!__private.showImage &&
						!S.AvatarPrivate.stringUnsuitableForInitials(avatarRoot.name)

				text: S.AvatarPrivate.initialsFromString(name)
				color: Kirigami.ColorUtils.brightnessForColor(__private.color) == Kirigami.ColorUtils.Light
					? "black"
					: "white"

				anchors.fill: parent
				font {
					pointSize: -1
					pixelSize: (avatarRoot.height - Kirigami.Units.largeSpacing) / 2
				}
				verticalAlignment: Text.AlignVCenter
				horizontalAlignment: Text.AlignHCenter
			}
			Kirigami.Icon {
				visible: (avatarRoot.initialsMode == Avatar.InitialsMode.UseIcon && !__private.showImage) ||
						(S.AvatarPrivate.stringUnsuitableForInitials(avatarRoot.name) && !__private.showImage)

				source: "user"

				anchors.fill: parent
				anchors.margins: Kirigami.Units.largeSpacing

				Kirigami.Theme.textColor: Kirigami.ColorUtils.brightnessForColor(__private.color) == Kirigami.ColorUtils.Light
										? "black"
										: "white"
			}
			Image {
				id: avatarImage
				visible: __private.showImage

				mipmap: true
				smooth: true
				sourceSize {
					width: avatarRoot.width
					height: avatarRoot.height
				}

				fillMode: Image.PreserveAspectFit
				anchors.fill: parent
			}

			Rectangle {
				color: "transparent"

				radius: width / 2
				anchors.fill: parent

				border {
					width: avatarRoot.border.width
					color: avatarRoot.border.color
				}
			}

			Rectangle {
				id: secondaryRect
				visible: false

				anchors {
					bottom: parent.bottom
					left: parent.left
					right: parent.right
				}

				height: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing*2

				color: Qt.rgba(0, 0, 0, 0.6)

				Kirigami.Icon {
					Kirigami.Theme.textColor: "white"
					source: (avatarRoot.actions.secondary || {iconName: ""}).iconName

					width: 16
					height: 16

					x: Math.round((parent.width/2)-(this.width/2))
					y: Math.round((parent.height/2)-(this.height/2))
				}
			}

			layer.enabled: true
			layer.effect: OpacityMask {
				maskSource: Rectangle {
					height: avatarRoot.height
					width: avatarRoot.width
					radius: height / 2
					color: "black"
					visible: false
				}
			}
		}
	}

	QtObject {
		id: __private
		// This property allows us to fall back to colour generation if
		// the root colour property is undefined.
		property color color: {
			if (!!avatarRoot.color) {
				return avatarRoot.color
			}
			return S.AvatarPrivate.colorsFromString(name)
		}
		property bool showImage: {
			return (avatarRoot.imageMode == Avatar.ImageMode.AlwaysShowImage) ||
					(avatarImage.status == Image.Ready && avatarRoot.imageMode == Avatar.ImageMode.AdaptiveImageOrInitals)
		}
	}
}
