import QtQuick 2.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.PageRoute {
    name: "channels"
    cache: true

    Kirigami.ScrollablePage {
        title: "Channels"

        Kirigami.Theme.colorSet: Kirigami.Theme.View

        actions.main: Kirigami.Action {
            icon.name: "list-add"
            onTriggered: sheety.open()
        }

        ListView {
            id: channelsView
            model: Kirigami.PageRouter.data

            delegate: Kirigami.SwipeListItem {
                contentItem: QQC2.Label {
                    text: `#${channelName}`
                    anchors.verticalCenter: parent.verticalCenter
                    verticalAlignment: Text.AlignVCenter
                }

                actions: [
                    Kirigami.Action {
                        icon.name: "edit-delete"
                        onTriggered: channelsView.model.deleteChannel(channelID)
                    }
                ]
            }
        }

        ChannelSheet {
            id: sheety
            model: channelsView.model
        }
    }
}
