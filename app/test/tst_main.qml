import com.github.HarmonyDevelopment.Challah.Tests 1.0
import QtTest 1.15
import QtQuick 2.0

Rectangle {
	id: t
	color: "green"

	function screenshot(name) {
		tcase.grabImage(rootWindow.contentItem).save(`/tmp/challah-test-screenshots/${name}.png`)
	}

	MainWindow {
		id: rootWindow
		testing: true

		TestCase {
			id: tcase
			when: windowShown

			function test_aa_wake_up() {
				wait(3*1000)
				t.screenshot("welcome")
			}
			function test_ab_get_to_login() {
				const txtField = findChild(rootWindow, "LoginRoute-Homeserver-TextField")
				txtField.text = "chat.harmonyapp.io:2289"

				const btn = findChild(rootWindow, "LoginRoute-Homeserver-Continue")
				btn.clicked()

				wait(5*1000)
				t.screenshot("auth-choices")
			}
			function test_ac_login_pages() {
				const loginButton = findChild(rootWindow, "LoginScreen-Choice-login")
				loginButton.clicked()

				wait(5*1000)
				t.screenshot("auth-login")
			}
		}
	}
}
