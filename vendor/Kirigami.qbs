StaticLibrary {
	name: "vendored_kirigami"

	files: [
		"kirigami/kirigami.qrc",
		"kirigami/src/scenegraph/shaders/shaders.qrc",
		"kirigami/src/*.cpp",
		"kirigami/src/*.h",
		"kirigami/src/scenegraph/*.cpp",
		"kirigami/src/scenegraph/*.h",
		"kirigami/src/libkirigami/*.cpp",
		"kirigami/src/libkirigami/*.h",
		"loggingcategory.cpp",
	]

	Qt.core.pluginMetaData: ["uri=org.kde.kirigami"]

	cpp.includePaths: [buildDirectory, sourceDirectory, "kirigami/src", "kirigami/src/libkirigami"]
	cpp.defines: ["QT_NO_CAST_FROM_ASCII", "QT_STATICPLUGIN"]

	Depends { name: "cpp" }
	Depends { name: "Qt"; submodules: ["core", "qml", "quick", "gui", "svg", "network", "quickcontrols2", "concurrent"] }
}
