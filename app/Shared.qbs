StaticLibrary {
	name: "ChallahShared"

	Export {
		Depends { name: "cpp" }

		cpp.defines: [].concat(project.vendoredKirigami ? ["CHALLAH_VENDORED_KIRIGAMI"] : []).concat(project.vendoredQQC2BreezeStyle ? ["CHALLAH_VENDORED_QQC2_BREEZE_STYLE"] : [])
	}

	cpp.defines: [].concat(project.vendoredKirigami ? ["CHALLAH_VENDORED_KIRIGAMI"] : []).concat(project.vendoredQQC2BreezeStyle ? ["CHALLAH_VENDORED_QQC2_BREEZE_STYLE"] : [])
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
		"gen/*/*/*.cpp",
		"gen/*/*/*.cc",
		"gen/*/*/*.h",
	].concat(
		project.vendoredKirigami ? ["../vendor/kirigami/kirigami.qrc"] : []
	)
	excludeFiles: [
		"tst.cpp",
		"main.cpp",
	]

	Depends { name: "bundle" }
	Depends { name: "cpp" }
	Depends { name: "vendored_protobuf"; condition: project.vendoredProtobuf }
	Depends { name: "vendored_kirigami"; condition: project.vendoredKirigami }
	Depends { name: "vendored_qqc2_breeze_style"; condition: project.vendoredQQC2BreezeStyle }
	Depends { name: "Qt"; submodules: ["gui", "network", "concurrent", "widgets", "websockets", "quick", "quickcontrols2", "qml"].concat(qbs.targetOS.contains("android") ? ["androidextras"] : []) }
}
