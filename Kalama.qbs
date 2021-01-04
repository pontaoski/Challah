Project {
	references: [
		"Source.qbs",
	]

	SubProject {
		filePath: "vendor/Kirigami.qbs"
		condition: qbs.targetOS.contains("android")
	}
	SubProject {
		filePath: "vendor/grpc.qbs"
		condition: qbs.targetOS.contains("android")
	}
}
