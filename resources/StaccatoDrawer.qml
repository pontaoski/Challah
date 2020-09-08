import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.10
import org.kde.kirigami 2.11 as Kirigami
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.GlobalDrawer {
    id: drawer

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    width: 72

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

    contentItem: ScrollView {
        id: scrollView

        Kirigami.Theme.inherit: true
        anchors.fill: parent
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.anchors {
            top: scrollView.top
            bottom: scrollView.bottom
        }

        Flickable {
            contentWidth: 72

            ColumnLayout {
                width: 72
                implicitWidth: 72

                Repeater {
                    model: HState.guildModel
                    delegate: Rectangle {
                        implicitWidth: 48
                        implicitHeight: 48
                        color: "cyan"
                        radius: 48 / 2

                        Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

                        ToolTip.text: model['guildName']
                        ToolTip.visible: maus.containsMouse

                        MouseArea {
                            id: maus
                            anchors.fill: parent
                            hoverEnabled: true

                            onClicked: root.router.navigateToRoute({"route": "channels", "data": model['channelModel'], "title": model['guildName']})
                        }
                    }
                }
            }
        }
    }

    modal: !drawer.wideScreen
    handleVisible: !drawer.wideScreen && shouldShow
}