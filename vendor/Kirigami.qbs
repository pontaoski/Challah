StaticLibrary {
	name: "kirigami"

	files: [
		"**.qrc",
		"src/**.cpp",
		"src/**.h",
	]

	cpp.includePaths: [buildDirectory, "src/libkirigami"]
	cpp.defines: ["QT_NO_CAST_FROM_ASCII", "KIRIGAMI_BUILD_TYPE_STATIC"]

	Depends { name: "cpp" }
	Depends { name: "Qt"; submodules: ["core", "qml", "quick", "gui", "svg", "network", "quickcontrols2", "concurrent"] }
}
