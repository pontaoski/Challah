StaticLibrary {
	name: "ChallahShared"

	Export {
		Group {
			condition: project.vendoredKirigami
			files: ["../vendor/kirigami/kirigami.qrc"]
		}

		Depends { name: "bundle" }
		Depends { name: "cpp" }
		Depends { name: "harmony-qt-sdk" }
		Depends { name: "gstreamer-1.0" }
		Depends { name: "gstreamer-webrtc-1.0" }
		Depends { name: "gstreamer-rtp-1.0" }
		Depends { name: "gstreamer-rtsp-1.0" }
		Depends { name: "gstreamer-sdp-1.0" }
		Depends { name: "json-glib-1.0" }
		Depends { name: "vendored_protobuf"; condition: project.vendoredProtobuf }
		Depends { name: "vendored_kirigami"; condition: project.vendoredKirigami }
		Depends { name: "android_openssl"; condition: qbs.targetOS.contains("android") }
		Depends { name: "vendored_qqc2_breeze_style"; condition: project.vendoredQQC2BreezeStyle }
		Depends { name: "Qt"; submodules: ["core", "core-private", "gui", "network", "concurrent", "widgets", "websockets", "quick", "quickcontrols2", "qml", "qml-private"].concat(qbs.targetOS.contains("android") ? ["androidextras"] : []) }

		cpp.defines: ["QT_NO_KEYWORDS"].concat(project.vendoredKirigami ? ["CHALLAH_VENDORED_KIRIGAMI"] : []).concat(project.vendoredQQC2BreezeStyle ? ["CHALLAH_VENDORED_QQC2_BREEZE_STYLE"] : [])
		cpp.cppFlags: ['-Werror=return-type']
		cpp.driverLinkerFlags: ['-lsdptransform']
		cpp.cxxLanguageVersion: "c++17"
		cpp.debugInformation: true
		cpp.separateDebugInformation: true
		cpp.enableExceptions: true
		cpp.enableReproducibleBuilds: true
		cpp.enableRtti: true
		cpp.includePaths: ["gen", "relationallib", "stores", "ui", "."]
	}

	cpp.defines: ["QT_NO_KEYWORDS"].concat(project.vendoredKirigami ? ["CHALLAH_VENDORED_KIRIGAMI"] : []).concat(project.vendoredQQC2BreezeStyle ? ["CHALLAH_VENDORED_QQC2_BREEZE_STYLE"] : [])
	cpp.cppFlags: ['-Werror=return-type']
	cpp.driverLinkerFlags: ['-lsdptransform']
	cpp.cxxLanguageVersion: "c++17"
	cpp.debugInformation: true
	cpp.separateDebugInformation: true
	cpp.enableExceptions: true
	cpp.enableReproducibleBuilds: true
	cpp.enableRtti: true

	debugInformationInstallDir: "bin"
	installDebugInformation: true

	cpp.includePaths: ["gen", "relationallib", "stores", "ui", "."]

	files: [
		"*.cpp",
		"*.h",
		"stores/*.cpp",
		"stores/*.h",
		"ui/*.cpp",
		"ui/*.h",
		"relationallib/*.cpp",
		"relationallib/*.h",
		"gen/*/*/*.cpp",
		"gen/*/*/*.cc",
		"gen/*/*/*.h",
	]
	excludeFiles: [
		"tst.cpp",
		"main.cpp",
	]

	Depends { name: "bundle" }
	Depends { name: "cpp" }
	Depends { name: "harmony-qt-sdk" }
	Depends { name: "gstreamer-1.0" }
	Depends { name: "gstreamer-webrtc-1.0" }
	Depends { name: "gstreamer-rtp-1.0" }
	Depends { name: "gstreamer-rtsp-1.0" }
	Depends { name: "gstreamer-sdp-1.0" }
	Depends { name: "json-glib-1.0" }
	Depends { name: "android_openssl"; condition: qbs.targetOS.contains("android") }
	Depends { name: "vendored_kirigami"; condition: project.vendoredKirigami }
	Depends { name: "vendored_qqc2_breeze_style"; condition: project.vendoredQQC2BreezeStyle }
	Depends { name: "Qt"; submodules: ["core", "core-private", "gui", "network", "concurrent", "widgets", "websockets", "quick", "quickcontrols2", "qml", "qml-private"].concat(qbs.targetOS.contains("android") ? ["androidextras"] : []) }
}
