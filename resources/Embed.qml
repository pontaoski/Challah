import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

QQC2.Control {
    leftPadding: Kirigami.Units.largeSpacing * 2
    rightPadding: Kirigami.Units.largeSpacing * 2
    topPadding: Kirigami.Units.largeSpacing * 2
    bottomPadding: Kirigami.Units.largeSpacing * 2

    background: Kirigami.ShadowedRectangle {
        radius: 5
        border {
            width: 2
            color: Qt.rgba(
                (((modelData.color >> 16) & 255) / 255),
                (((modelData.color >> 8) & 255) / 255),
                ((modelData.color & 255) / 255),
                1
            )
        }
        color: Kirigami.Theme.backgroundColor
    }
    contentItem: ColumnLayout {
        Kirigami.Heading {
            level: 3
            text: modelData.title
            wrapMode: Text.Wrap

            Layout.fillWidth: true
        }
        QQC2.Label {
            text: modelData.body
            wrapMode: Text.Wrap

            Layout.fillWidth: true
        }

        Repeater {
            model: modelData.actions
            delegate: MessageAction {
                messageID: messageDelegate.modelMessageID
            }
        }
    }

    Layout.fillWidth: true
}
