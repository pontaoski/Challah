import QtQuick 2.10
import QtQuick.Window 2.10
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import com.github.HarmonyDevelopment.Staccato 1.0

Kirigami.PageRoute {
    name: "login"
    cache: false

    Kirigami.Page {
        title: "Login"

        Kirigami.Theme.colorSet: Kirigami.Theme.View

        Kirigami.FormLayout {
            anchors.fill: parent

            QQC2.TextField {
                id: email
                text: "r@r.r"
                Kirigami.FormData.label: "email:"
            }
            QQC2.TextField {
                id: homeserver
                text: "localhost:2289"
                Kirigami.FormData.label: "homeserver:"
            }
            Kirigami.PasswordField {
                id: password
                text: "10kekeAke"
                Kirigami.FormData.label: "password:"
            }
            QQC2.Button {
                text: "Login"
                onClicked: {
                    if (HState.login(email.text, password.text, homeserver.text)) {
                        root.showPassiveNotification("Logged in")
                        Kirigami.PageRouter.navigateToRoute("no-guild")
                        root.globalDrawer.shouldShow = true
                    } else {
                        root.showPassiveNotification("Failed to log in")
                    }
                }
            }
        }
    }
}