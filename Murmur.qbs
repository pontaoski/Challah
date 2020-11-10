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
