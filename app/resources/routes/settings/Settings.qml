// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.10
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.14 as Kirigami
import org.kde.kitemmodels 1.0

Kirigami.ApplicationWindow {
    visible: false

    title: qsTr("Preferences")

    width: Kirigami.Units.gridUnit * 40
    height: Kirigami.Units.gridUnit * 25

    header: Kirigami.Separator {
        anchors {
            left: parent.left
            right: parent.right
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        QQC2.ScrollView {
            Layout.fillHeight: true

            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View

            background: Rectangle {
                color: Kirigami.Theme.backgroundColor
            }

            ColumnLayout {
                spacing: 0

                SidebarItem {
                    pageName: qsTr("Profiles")
                    accessibleDescription: qsTr("Open profiles settings")
                    icon.name: "preferences-system-users"
                    page: Qt.resolvedUrl("OverrideSettings.qml")
                }
            }
        }
        Kirigami.Separator {
            Layout.fillHeight: true
        }
        Loader {
            id: pageLoader
            source: Qt.resolvedUrl("OverrideSettings.qml")
            asynchronous: true
            onStatusChanged: if (this.status == Loader.Error) Qt.quit(1)

            Layout.margins: item && item.noMargin ? 0 : Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
