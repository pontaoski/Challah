import qbs.FileInfo

QtApplication {
	name: "Challah"

	cpp.cppFlags: ['-Werror=return-type']
	cpp.cxxLanguageVersion: "c++17"
	cpp.debugInformation: true
	cpp.separateDebugInformation: true
	cpp.enableExceptions: true
	cpp.enableReproducibleBuilds: true
	cpp.enableRtti: true
	cpp.defines: iconsDir.length > 0 ? ["CHALLAH_BUNDLED_ICONS=1"] : []

	debugInformationInstallDir: "bin"
	installDebugInformation: true

	property string iconsDir: ""

	files: [
		"main.cpp",
	]

	Group {
		condition: product.iconsDir.length > 0
		files: [product.iconsDir]
		fileTags: "qt.core.resource_data"
		Qt.core.resourceSourceBase: product.iconsDir.slice(0, -FileInfo.baseName(product.iconsDir).length)
		Qt.core.resourcePrefix: "/"
	}

	Group {
		files: ["resources/**"]
		excludeFiles: ["resources/img/io.harmonyapp.Challah.svg"]
		fileTags: "qt.core.resource_data"
		Qt.core.resourceSourceBase: "resources/"
		Qt.core.resourcePrefix: "/"
	}

	Group {
		files: ["resources/img/io.harmonyapp.Challah.svg"]
		fileTags: "qt.core.resource_data"
		qbs.install: qbs.targetOS.contains("linux")
		qbs.installDir: "share/icons/hicolor/scalable/apps"
		Qt.core.resourceSourceBase: "resources/"
		Qt.core.resourcePrefix: "/"
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

	Properties {
		condition: qbs.targetOS.contains("android")
		Android.sdk.packageName: "com.github.HarmonyDevelopment.Challah"
	}

	Depends { name: "harmony-qt-sdk" }
	Depends { name: "HarmonyProtocol" }
	Depends { name: "ChallahShared" }
}
