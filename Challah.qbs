Project {
	references: [
		"app/Source.qbs",
		"app/Test.qbs",
	]

	property bool vendoredProtobuf: false
	property bool vendoredKirigami: false

	AutotestRunner { }

	SubProject {
		filePath: "vendor/Kirigami.qbs"
		condition: project.vendoredKirigami
	}
	SubProject {
		filePath: "vendor/protobuf.qbs"
		condition: project.vendoredProtobuf
	}
}
