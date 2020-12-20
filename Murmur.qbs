QtApplication {
	name: "Murmur"

	protobuf.cpp.useGrpc: true

	cpp.cppFlags: ['-Werror=return-type']
	cpp.cxxLanguageVersion: "c++17"

	files: [
		"*.cpp",
		"*.hpp",
		"resources/data.qrc"
	]

	Group {
		files: ["resources/com.github.harmony-development.Murmur.svg"]
		qbs.install: qbs.targetOS.contains("linux")
		qbs.installDir: "share/icons/hicolor/scalable/apps"
	}

	Group {
		files: ["com.github.harmony-development.Murmur.appdata.xml"]
		qbs.install: qbs.targetOS.contains("linux")
		qbs.installDir: "share/metainfo"
	}

	Group {
		files: ["com.github.harmony-development.Murmur.desktop"]
		qbs.install: qbs.targetOS.contains("linux")
		qbs.installDir: "share/applications"
	}

	qbs.install: qbs.targetOS.contains("linux")
	qbs.installDir: "bin"

	Group {
		name: "Translation files"
		files: ["po/*.ts"]
	}
	Group {
		name: "QRC"
		fileTagsFilter: "qm"
		fileTags: "qt.core.resource_data"
		Qt.core.resourcePrefix: "/po"
	}

	Group {
		files: [
			"protocol/core/v1/core.proto",
			"protocol/foundation/v1/foundation.proto",
			"protocol/profile/v1/profile.proto"
		]
		fileTags: "protobuf.grpc"
	}

	Depends { name: "cpp" }
	Depends { name: "protobuf.cpp" }
	Depends { name: "Qt"; submodules: ["gui", "concurrent", "widgets", "quick", "quickcontrols2", "qml"] }
}
