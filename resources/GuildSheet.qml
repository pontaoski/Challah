import QtQuick 2.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.OverlaySheet {
    id: rooty

    Kirigami.FormLayout {
        QQC2.TextField {
            id: texty

            Kirigami.FormData.label: "Guild Name:"
        }
        QQC2.Button {
            text: "Create Guild"

            onClicked: {
                if (HState.createGuild(texty.text)) {
                    root.showPassiveNotification("Created guild")
                } else {
                    root.showPassiveNotification("Failed to make guild")
                }
                rooty.close()
            }
        }
    }
}