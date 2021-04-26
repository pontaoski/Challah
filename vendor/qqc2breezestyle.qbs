StaticLibrary {
	name: "vendored_qqc2_breeze_style"

	files: [
		"qqc2-breeze-style/style/*.cpp",
		"qqc2-breeze-style/style/*.h",
		"qqc2-breeze-style/style/impl/*.cpp",
		"qqc2-breeze-style/style/impl/*.h",
	]

	Export {
		Group {
			files: [
				"qqc2-breeze-style/style/qtquickcontrols/*.qml",
				"qqc2-breeze-style/style/qtquickcontrols/qmldir",
			]
			fileTags: "qt.core.resource_data"
			Qt.core.resourcePrefix: "/org/kde/breeze"
		}
		Group {
			files: [
				"qqc2-breeze-style/style/impl/*.qml",
				"qqc2-breeze-style/style/impl/qmldir",
			]
			fileTags: "qt.core.resource_data"
			Qt.core.resourcePrefix: "/org/kde/breeze/impl"
		}
	}

	Qt.core.pluginMetaData: ["uri=org.kde.breeze"]

	cpp.includePaths: [buildDirectory, sourceDirectory, "qqc2-breeze-style/style", "qqc2-breeze-style/style/impl"]
	cpp.defines: ["QT_NO_CAST_FROM_ASCII", "QT_STATICPLUGIN"]

	Depends { name: "cpp" }
	Depends { name: "vendored_kguiaddons" }
	Depends { name: "Qt"; submodules: ["core", "qml", "quick", "gui", "svg", "network", "quickcontrols2", "concurrent"] }
}
