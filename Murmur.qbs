QtApplication {
	name: "Murmur"

	protobuf.cpp.importPaths: ["protocol"]
	protobuf.cpp.useGrpc: true

	cpp.cppFlags: ['-Werror=return-type']
	cpp.cxxLanguageVersion: "c++17"
	cpp.debugInformation: true
	cpp.separateDebugInformation: true
	cpp.enableExceptions: true
	cpp.enableReproducibleBuilds: true
	cpp.enableRtti: true

	debugInformationInstallDir: "bin"
	installDebugInformation: true

	files: [
		"*.cpp",
		"*.hpp",
		"resources/data.qrc"
	]

	Group {
		files: ["resources/io.harmonyapp.Murmur.svg"]
		qbs.install: qbs.targetOS.contains("linux")
		qbs.installDir: "share/icons/hicolor/scalable/apps"
	}

	Group {
		files: ["io.harmonyapp.Murmur.appdata.xml"]
		qbs.install: qbs.targetOS.contains("linux")
		qbs.installDir: "share/metainfo"
	}

	Group {
		files: ["io.harmonyapp.Murmur.desktop"]
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
			"protocol/auth/v1/auth.proto",
			"protocol/chat/v1/channels.proto",
			"protocol/chat/v1/chat.proto",
			"protocol/chat/v1/emotes.proto",
			"protocol/chat/v1/guilds.proto",
			"protocol/chat/v1/messages.proto",
			"protocol/chat/v1/permissions.proto",
			"protocol/chat/v1/profile.proto",
			"protocol/chat/v1/streaming.proto",
			"protocol/harmonytypes/v1/types.proto",
		]
		fileTags: "protobuf.grpc"
	}

	Depends { name: "cpp" }
	Depends { name: "protobuf.cpp" }
	Depends { name: "Qt"; submodules: ["gui", "concurrent", "widgets", "quick", "quickcontrols2", "qml"] }
}
