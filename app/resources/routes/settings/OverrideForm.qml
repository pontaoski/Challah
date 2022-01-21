// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QQC2
import QtQuick.Window 2.2
import Qt.labs.qmlmodels 1.0

import org.kde.kirigami 2.12 as Kirigami

Kirigami.FormLayout {
    id: formLayout

    width: 600

    property alias data_display: display.text
    property alias data_avatar: avatar.text
    property alias data_before: before.text
    property alias data_after: after.text

    function reset() {
        data_display = ""
        data_before = ""
        data_after = ""
    }

    QQC2.TextField {
        id: display
        Layout.maximumWidth: 200
        Kirigami.FormData.label: qsTr("Display name:")
    }
    RowLayout {
        Kirigami.FormData.label: qsTr("Tags:")

        Layout.maximumWidth: 200
        QQC2.TextField {
            id: before
            Layout.maximumWidth: 60
        }
        QQC2.Label {
            text: qsTr(":text:")
        }
        QQC2.TextField {
            id: after
            Layout.maximumWidth: 60
        }
    }
    QQC2.Label {
        font: Kirigami.Theme.smallFont
        text: qsTr(`Sending a message with text surrounded by "%1" and "%2" will send it using this profile`).arg(before.text).arg(after.text)
        wrapMode: Text.Wrap
        Layout.maximumWidth: 300
        Layout.fillWidth: true
    }
    RowLayout {
        Kirigami.FormData.label: qsTr("Avatar URL:")

        QQC2.TextField {
            id: avatar
        }
    }
}