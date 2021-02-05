Project {
	references: [
		"app/Source.qbs",
	]

	property bool vendored: false

	SubProject {
		filePath: "vendor/Kirigami.qbs"
		condition: project.vendored
	}
	SubProject {
		filePath: "vendor/protobuf.qbs"
		condition: project.vendored
	}
}
