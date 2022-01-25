// SPDX-FileCopyrightText: 2022 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2

Kirigami.BasicListItem {
    trailing: Item {
        Kirigami.Icon {
            source: "arrow-right"
            height: Kirigami.Units.iconSizes.small
            width: height

            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
            }
        }
    }
}