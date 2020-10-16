// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.10
import org.kde.kirigami 2.13 as Kirigami
import com.github.HarmonyDevelopment.Staccato 1.0

Item {
	id: drawer

	implicitWidth: model !== null ? 200 : 0

	Kirigami.Theme.inherit: true
	Kirigami.Theme.colorSet: Kirigami.Theme.View

	Rectangle {
		anchors.fill: parent

		color: Kirigami.Theme.backgroundColor

		Kirigami.Separator {
			anchors {
				top: parent.top
				left: parent.left
				bottom: parent.bottom
			}
		}
	}

	property var model: null

	ColumnLayout {
		spacing: 0
		anchors.fill: parent

		Kirigami.ApplicationHeader {
			z: 2
			Layout.fillWidth: true

			contentItem: RowLayout {
				anchors {
					verticalCenter: parent.verticalCenter
					left: parent.left
					right: parent.right
				}

				Kirigami.Heading {
					leftPadding: Kirigami.Units.largeSpacing
					text: "Guild Info"
					Layout.fillWidth: true
				}
			}
			pageDelegate: Item {}
		}

		ListView {
			model: drawer.model

			Kirigami.Theme.inherit: true
			Kirigami.Theme.colorSet: Kirigami.Theme.View

			Layout.fillWidth: true
			Layout.fillHeight: true

			delegate: Kirigami.AbstractListItem {
				hoverEnabled: false
				highlighted: false
				contentItem: RowLayout {
					Kirigami.Avatar {
						name: display
						source: decoration

						Layout.preferredWidth: Math.floor(Kirigami.Units.gridUnit * 1.75)
						Layout.preferredHeight: Math.floor(Kirigami.Units.gridUnit * 1.75)
					}
					Label {
						text: display
						verticalAlignment: Text.AlignVCenter

						Layout.fillWidth: true
						Layout.fillHeight: true
					}
				}
			}
		}
	}

}
