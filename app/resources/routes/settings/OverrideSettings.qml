// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QQC2
import QtQuick.Window 2.2
import org.kde.kitemmodels 1.0
import com.github.HarmonyDevelopment.Challah 1.0

import org.kde.kirigami 2.15 as Kirigami

Item {
    id: overridesSettings

    property var model: CState.overridesModel

    ColumnLayout {
        id: form

        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        QQC2.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: false

            Kirigami.Theme.colorSet: Kirigami.Theme.View
            QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

            background: Rectangle {
                color: Kirigami.Theme.backgroundColor
                border.width: 1
                border.color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.3)
                radius: 3
                anchors.fill: parent
                anchors.margins: -1
            }

            ListView {
                clip: true
                model: KSortFilterProxyModel {
                    sourceModel: overridesSettings.model
                }
                delegate: Kirigami.BasicListItem {
                    required property string username

                    text: username
                }
            }
        }

        Kirigami.ApplicationWindow {
            id: addOverrideWindow
            visible: false

            title: qsTr("Add New Profile")

            function doOpen() {
                this.visible = true
                formLayout.reset()
            }
            function doClose(ok) {
                this.visible = false
                if (!ok) {
                    return
                }
                overridesSettings.model.addOverride(formLayout.data_display, "", formLayout.data_before, formLayout.data_after)
            }

            width: 600
            minimumWidth: 600
            maximumWidth: 600

            height: formLayout.implicitHeight + Kirigami.Units.gridUnit * 4
            minimumHeight: formLayout.implicitHeight + Kirigami.Units.gridUnit * 4
            maximumHeight: formLayout.implicitHeight + Kirigami.Units.gridUnit * 4

            OverrideForm {
                id: formLayout
            }

            footer: QQC2.ToolBar {
                position: QQC2.ToolBar.Footer
                contentItem: RowLayout {
                    Item { Layout.fillWidth: true }
                    QQC2.Button {
                        text: qsTr("Cancel")
                        onClicked: addOverrideWindow.doClose(false)
                    }
                    QQC2.Button {
                        text: qsTr("Add Profile")
                        onClicked: addOverrideWindow.doClose(true)
                    }
                }
            }

            header: Kirigami.Separator {
                anchors {
                    left: parent.left
                    right: parent.right
                }
            }
        }

        RowLayout {
            QQC2.Button {
                icon.name: "add"
                text: qsTr("Add...")
                onClicked: addOverrideWindow.doOpen()
            }
            Item { Layout.fillWidth: true }
            QQC2.Button {
                enabled: overridesSettings.model.dirty
                text: qsTr("Apply")
                onClicked: overridesSettings.model.save()
            }
            Layout.fillWidth: true
        }
    }

    Layout.fillHeight: true
    Layout.fillWidth: true
}
