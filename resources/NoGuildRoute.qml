import QtQuick 2.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.PageRoute {
    name: "no-guild"
    cache: false

    Kirigami.Page {
        title: "Guilds"

        globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None

        Kirigami.Theme.colorSet: Kirigami.Theme.View

        Kirigami.PlaceholderMessage {
            text: "Click a guild to get chatting."
            icon.name: "view-conversation-balloon"

            anchors.centerIn: parent

            helpfulAction: Kirigami.Action {
                iconName: "list-add"
                text: "Join or Create Guild..."
                onTriggered: guildSheet.openAndClear()
            }
        }
    }
}