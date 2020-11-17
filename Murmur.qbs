import qbs.FileInfo
import qbs.ModUtils
import qbs.TextFile
import qbs.Utilities
import qbs.Xml

QtApplication {
	name: "Murmur"

	protobuf.cpp.useGrpc: true
	protobuf.cpp.libraryPath: "/usr/lib64/libprotobuf.so.22"
	protobuf.cpp.grpcLibraryPath: "/usr/lib64/libgrpc++.so.1"
	protobuf.cpp.grpcIncludePath: "/usr/include/grpc++"
	protobuf.cpp.includePath: "/usr/include/google/protobuf"

	cpp.cppFlags: ['-Werror=return-type']
	cpp.cxxLanguageVersion: "c++17"

	files: [
		"main.cpp",
		"state.cpp",
		"client.cpp",
		"guild.cpp",
		"channels.cpp",
		"messages.cpp",
		"invites.cpp",
		"overlappingpanels.cpp",
		"state.hpp",
		"client.hpp",
		"guild.hpp",
		"channels.hpp",
		"util.cpp",
		"util.hpp",
		"messages.hpp",
		"invites.hpp",
		"overlappingpanels.hpp",
		"avatar.cpp",
		"avatar.hpp",
		"resources/data.qrc"
	]

	Group {
		files: ["resources/com.github.harmony-development.Murmur.svg"]
		qbs.install: qbs.targetOS.contains("linux")
		qbs.installDir: "share/icons/hicolor/scalable/apps"
	}

	Group {
		files: ["com.github.harmony-development.Murmur.appdata.xml"]
		qbs.install: qbs.targetOS.contains("linux")
		qbs.installDir: "share/metainfo"
	}

	Group {
		files: ["com.github.harmony-development.Murmur.desktop"]
		qbs.install: qbs.targetOS.contains("linux")
		qbs.installDir: "share/applications"
	}

	qbs.install: qbs.targetOS.contains("linux")
	qbs.installDir: "bin"

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

	Group {
		files: [
			"protocol/core/v1/core.proto",
			"protocol/foundation/v1/foundation.proto",
			"protocol/profile/v1/profile.proto"
		]
		fileTags: "protobuf.grpc"
	}

	Depends { name: "cpp" }
	Depends { name: "protobuf.cpp" }
	Depends { name: "Qt"; submodules: ["gui", "concurrent", "widgets", "quick", "quickcontrols2", "qml"] }
}
