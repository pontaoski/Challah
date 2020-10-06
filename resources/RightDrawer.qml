import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.10
import org.kde.kirigami 2.13 as Kirigami
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.ContextDrawer {
    id: drawer

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    width: 300

    Kirigami.Theme.inherit: true
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    property var model: {}
    property bool shouldShow: false
    property bool wideScreen: false
    onWideScreenChanged: {
        if (wideScreen) {
            drawerOpen = Qt.binding(function() { return shouldShow })
        } else {
            drawerOpen = false
        }
    }
    drawerOpen: shouldShow && wideScreen

    contentItem: ColumnLayout {
        spacing: 0

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

            Layout.fillWidth: true
            Layout.fillHeight: true

            delegate: Kirigami.AbstractListItem {
                hoverEnabled: false
                highlighted: false
                contentItem: RowLayout {
                    Kirigami.Avatar {
                        name: display
                        source: decoration

                        Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 2
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

    modal: !drawer.wideScreen
    handleVisible: !drawer.wideScreen && shouldShow
}
