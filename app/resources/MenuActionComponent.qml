// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Challah 1.0

QQC2.Menu {
    required property var menuContent

    Repeater {
        model: menuContent

        Loader {
            active: !!modelData.children
            source: "RecursiveMenuActionComponent.qml"
        }

        Loader {
            active: !modelData.children

            QQC2.MenuItem {
                text: modelData.text

                onTriggered: rootAction.triggerAction(modelData["id"], "")
            }
        }
    }
}
