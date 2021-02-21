QtApplication {
	name: "tst_challah"
	type: ["application", "autotest"]

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
		"tst.cpp",
		"resources/data.qrc",
	]

	Group {
		files: ["test/tst_*.qml"]
		qbs.install: true
		qbs.installDir: "bin"
	}

	install: qbs.targetOS.contains("linux")
	installDir: "bin"

	Depends { name: "ChallahShared" }
	Depends { name: "Qt"; submodules: ["gui", "qmltest", "concurrent", "widgets", "websockets", "quick", "quickcontrols2", "qml"] }
}
