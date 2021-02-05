Project {
	references: [
		"app/Source.qbs",
	]

	property bool vendoredProtobuf: false
	property bool vendoredKirigami: false

	SubProject {
		filePath: "vendor/Kirigami.qbs"
		condition: project.vendoredKirigami
	}
	SubProject {
		filePath: "vendor/protobuf.qbs"
		condition: project.vendoredProtobuf
	}
}
