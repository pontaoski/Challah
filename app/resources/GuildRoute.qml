// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.PageRoute {
    name: "guild"
    cache: false

    Kirigami.Page {
        title: qsTr("")

        globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None

        Kirigami.Theme.colorSet: Kirigami.Theme.View

        Kirigami.PlaceholderMessage {
            text: qsTr("Select a channel to get chatting.")
            icon.name: "view-conversation-balloon"

            anchors.centerIn: parent
        }
    }
}
