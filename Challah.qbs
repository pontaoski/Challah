Project {
	references: [
		"app/App.qbs",
		"app/Shared.qbs",
		"vendor/Chometz/Chometz.qbs",
	]

	property bool vendoredKirigami: false
	property bool vendoredQQC2BreezeStyle: false
	property bool withTests: false

	AutotestRunner { }

	SubProject {
		filePath: "app/Test.qbs"
		condition: project.withTests
	}
	SubProject {
		filePath: "vendor/Kirigami.qbs"
		condition: project.vendoredKirigami
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
