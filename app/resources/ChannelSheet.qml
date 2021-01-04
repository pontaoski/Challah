// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.OverlaySheet {
    id: rooty

	parent: applicationWindow().overlay
    property var model: null

    Kirigami.FormLayout {
        QQC2.TextField {
            id: texty

            Kirigami.FormData.label: qsTr("Channel Name:")
        }
        QQC2.Button {
            text: qsTr("Create Channel")

            onClicked: {
				rooty.model.createChannel(
					texty.text,
					//: the channel has been successfully created
					function() { root.showPassiveNotification(qsTr("Created channel")); rooty.close() },
					//: the channel failed to be created
					function() { root.showPassiveNotification(qsTr("Failed to create channel")); rooty.close() },
				)
            }
        }
    }
}
