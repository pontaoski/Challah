Project {
	references: [
		"app/App.qbs",
		"app/Shared.qbs",
		"app/Test.qbs",
	]

	property bool vendoredProtobuf: false
	property bool vendoredKirigami: false
	property bool vendoredQQC2BreezeStyle: false

	AutotestRunner { }

	SubProject {
		filePath: "vendor/Kirigami.qbs"
		condition: project.vendoredKirigami
	}
	SubProject {
		filePath: "vendor/protobuf.qbs"
		condition: project.vendoredProtobuf
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
