QtApplication {
	name: "Challah"

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
		"main.cpp",
		"resources/data.qrc",
	]

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

	Depends { name: "ChallahShared" }
	Depends { name: "Qt"; submodules: ["gui", "concurrent", "widgets", "websockets", "quick", "quickcontrols2", "qml"] }
}
