import QtQuick 2.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami

Kirigami.RouterWindow {
    id: root

    initialRoute: "login"

    pageStack.globalToolBar.showNavigationButtons: 0

    globalDrawer: StaccatoDrawer { wideScreen: root.wideScreen }

    LoginRoute {}
    NoGuildRoute {}
    ChannelRoute {}
    MessagesRoute {}
}