QtApplication {
	name: "Challah"

	protobuf.cpp.importPaths: ["protocol"]
	// protobuf.cpp.linkLibraries: !project.vendoredProtobuf

	cpp.defines: project.vendoredKirigami ? ["CHALLAH_VENDORED_DEPS"] : []
	cpp.cppFlags: ['-Werror=return-type']
	cpp.cxxLanguageVersion: "c++17"
	cpp.debugInformation: true
	cpp.separateDebugInformation: true
	cpp.enableExceptions: true
	cpp.enableReproducibleBuilds: true
	cpp.enableRtti: true

	debugInformationInstallDir: "bin"
	installDebugInformation: true

	cpp.includePaths: ["gen"]

	files: [
		"*.cpp",
		"*.hpp",
		"gen/chat/v1/*.cpp",
		"gen/auth/v1/*.cpp",
		"gen/mediaproxy/v1/*.cpp",
		"gen/voice/v1/*.cpp",
		"gen/chat/v1/*.h",
		"gen/auth/v1/*.h",
		"gen/mediaproxy/v1/*.h",
		"gen/voice/v1/*.h",
		"resources/data.qrc"
	].concat(
		project.vendoredKirigami ? ["../vendor/kirigami/kirigami.qrc"] : []
	)

	Group {
		files: ["resources/io.harmonyapp.Challah.svg"]
		qbs.install: qbs.targetOS.contains("linux")
		qbs.installDir: "share/icons/hicolor/scalable/apps"
	}

	Group {
		files: ["io.harmonyapp.Challah.appdata.xml"]
		qbs.install: qbs.targetOS.contains("linux")
		qbs.installDir: "share/metainfo"
	}

	Group {
		files: ["io.harmonyapp.Challah.desktop"]
		qbs.install: qbs.targetOS.contains("linux")
		qbs.installDir: "share/applications"
	}

	install: qbs.targetOS.contains("linux")
	installDir: "bin"

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
			"protocol/chat/v1/postbox.proto",
			"protocol/voice/v1/voice.proto",
			"protocol/harmonytypes/v1/types.proto",
			"protocol/mediaproxy/v1/mediaproxy.proto",
		]
		fileTags: "protobuf.input"
	}

	Depends { name: "bundle" }
	Depends { name: "cpp" }
	Depends { name: "protobuf.cpp" }
	Depends { name: "vendored_protobuf"; condition: project.vendoredProtobuf }
	Depends { name: "vendored_kirigami"; condition: project.vendoredKirigami }
	Depends { name: "Qt"; submodules: ["gui", "concurrent", "widgets", "websockets", "quick", "quickcontrols2", "qml"] }
}
