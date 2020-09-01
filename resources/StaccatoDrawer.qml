import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1
import org.kde.kirigami 2.11 as Kirigami

Kirigami.GlobalDrawer {
    id: drawer

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    width: Kirigami.Units.gridUnit * 15

    property bool wideScreen: false
    onWideScreenChanged: if (!wideScreen) drawerOpen = true

    header: Kirigami.AbstractApplicationHeader {
        id: toolbar
        visible: drawer.wideScreen

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Kirigami.Units.largeSpacing
            anchors.rightMargin: Kirigami.Units.largeSpacing

            Kirigami.Heading {
                text: "Guilds"
            }
        }
    }

    ColumnLayout {
        spacing: 0
        Layout.fillWidth: true
        Layout.leftMargin: -drawer.leftPadding
        Layout.rightMargin: -drawer.rightPadding

        Repeater {
            model: 20
            delegate: Rectangle {
                implicitWidth: 24
                implicitHeight: 24
                color: "cyan"
            }
        }
    }

    modal: !drawer.wideScreen
    handleVisible: !drawer.wideScreen
}