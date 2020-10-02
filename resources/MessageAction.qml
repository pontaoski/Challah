import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Loader {
    Component {
        id: buttonComponent

        RowLayout {
            QQC2.Button {
                text: modelData.text

                icon.name: modelData.url ? "link" : ""

                Kirigami.Theme.textColor: {
                    switch (modelData["type"]) {
                    case "Normal":
                        return Kirigami.Theme.textColor
                    case "Primary":
                        return Kirigami.Theme.positiveTextColor
                    case "Destructive":
                        return Kirigami.Theme.negativeTextColor
                    }
                }
                Kirigami.Theme.backgroundColor: {
                    switch (modelData["type"]) {
                    case "Normal":
                        return Kirigami.Theme.backgroundColor
                    case "Primary":
                        return Kirigami.Theme.positiveBackgroundColor
                    case "Destructive":
                        return Kirigami.Theme.negativeBackgroundColor
                    }
                }

                onClicked: {
                    if (modelData.url) {
                        Qt.openUrlExternally(modelData.url)
                    }
                }

                Layout.fillWidth: true
            }
            QQC2.Button {
                visible: !!modelData.children
                icon.name: "usermenu-down"
                onClicked: menuComponent.open()

                MenuActionComponent {
                    id: menuComponent
                    menuContent: modelData.children || {}
                }
            }
        }
    }
    Component {
        id: smallEntryComponent

        RowLayout {
            QQC2.TextField {
                Layout.fillWidth: true
            }
            QQC2.Button {
                text: "Submit"
            }
        }
    }
    Component {
        id: largeEntryComponent

        ColumnLayout {
            QQC2.TextArea {
                Layout.fillWidth: true
            }
            QQC2.Button {
                text: "Submit"

                Layout.alignment: Qt.AlignBottom | Qt.AlignRight
            }
        }
    }
    Component {
        id: dropdownComponent

        RowLayout {
            QQC2.ComboBox {
                model: modelData.children
                textRole: "text"

                Layout.fillWidth: true
            }
            QQC2.Button {
                text: "Submit"
            }
        }
    }

    sourceComponent: {
        switch (modelData.presentation || "") {
        case "Button":
        case "Menu":
        case "":
            return buttonComponent
        case "SmallEntry":
            return smallEntryComponent
        case "LargeEntry":
            return largeEntryComponent
        case "Dropdown":
            return dropdownComponent
        }
    }

    Layout.fillWidth: true
}
