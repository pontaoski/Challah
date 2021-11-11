QtApplication {
	name: "GStreamerTest"

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
		"gstreamer_main.cpp",
	]

	Depends { name: "ChallahShared" }
}
