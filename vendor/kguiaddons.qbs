StaticLibrary {
	name: "vendored_kguiaddons"

	files: [
		"kguiaddons/src/colors/*.cpp",
		"kguiaddons/src/colors/*.h",
	]

	Export {
		Depends { name: "cpp" }
		cpp.includePaths: [exportingProduct.buildDirectory, exportingProduct.sourceDirectory, "kguiaddons/src/colors"]
	}

	cpp.includePaths: [buildDirectory, sourceDirectory, "kguiaddons/src/colors"]
	cpp.defines: ["QT_NO_CAST_FROM_ASCII", "QT_STATICPLUGIN"]

	Depends { name: "cpp" }
	Depends { name: "Qt"; submodules: ["core", "qml", "quick", "gui", "svg", "network", "quickcontrols2", "concurrent"] }
}
