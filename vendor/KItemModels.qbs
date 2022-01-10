StaticLibrary {
	name: "vendored_kitemmodels"

	files: [
		"kitemmodels_debug.cpp",
		"kitemmodelsqml_logdeprecated.cpp",
		"kitemmodels/src/core/*.cpp",
		"kitemmodels/src/core/*.h",
		"kitemmodels/src/qml/*.cpp",
		"kitemmodels/src/qml/*.h",
		"kitemmodels/src/qml/kitemmodelsqml.qrc",
		"kitemmodels/src/qml/qmldir",
	]

	Qt.core.pluginMetaData: ["uri=org.kde.kitemmodels"]

	cpp.cxxLanguageVersion: "c++17"
	cpp.includePaths: [buildDirectory, sourceDirectory, "kitemmodels/src/core", "kitemmodels/src/qml"]
	cpp.defines: ["QT_NO_CAST_FROM_ASCII", "QT_STATICPLUGIN"]

	Depends { name: "cpp" }
	Depends { name: "Qt"; submodules: ["core", "qml"] }
}
