Project {
	references: [
		"app/App.qbs",
		"app/Shared.qbs",
		"vendor/Chometz/Chometz.qbs",
	]

	property bool enableVoice: false
	property bool vendoredKirigami: false
	property bool vendoredKItemModels: false
	property bool vendoredQQC2BreezeStyle: false
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
		filePath: "vendor/Kirigami.qbs"
		condition: project.vendoredKirigami
	}
	SubProject {
		filePath: "vendor/KItemModels.qbs"
		condition: project.vendoredKItemModels
	}
	SubProject {
		filePath: "vendor/android_openssl.qbs"
		condition: qbs.targetOS.contains("android")
	}
	SubProject {
		filePath: "vendor/qqc2breezestyle.qbs"
		condition: project.vendoredQQC2BreezeStyle
	}
	SubProject {
		filePath: "vendor/kguiaddons.qbs"
		condition: project.vendoredQQC2BreezeStyle
	}
}
