Project {
	references: [
		"app/App.qbs",
		"app/Shared.qbs",
		"vendor/Chometz/Chometz.qbs",
	]

	property bool enableVoice: false
	property bool withTests: false

	AutotestRunner { }

	SubProject {
		filePath: "app/GStreamerTest.qbs"
		condition: project.enableVoice
	}
	SubProject {
		filePath: "app/Test.qbs"
		condition: project.withTests
	}
	SubProject {
		filePath: "vendor/android_openssl.qbs"
		condition: qbs.targetOS.contains("android")
	}
}
