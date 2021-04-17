StaticLibrary {
	name: "vendored_protobuf"

	files: [
		"protobuf/src/google/protobuf/*.cc",
		"protobuf/src/google/protobuf/io/*.cc",
		"protobuf/src/google/protobuf/util/*.cc",
		"protobuf/src/google/protobuf/stubs/*.cc",
		"protobuf/src/google/protobuf/util/internal/*.cc",
	]
	excludeFiles: [
		"protobuf/src/google/protobuf/io/*test.cc",
		"protobuf/src/google/protobuf/util/*test.cc",
		"protobuf/src/google/protobuf/util/internal/*test.cc",
		"protobuf/src/google/protobuf/stubs/*test.cc",
		"protobuf/src/google/protobuf/*_unittest.cc",
		"protobuf/src/google/protobuf/*_test.cc",
		"protobuf/src/google/protobuf/*_test_util.cc",
		"protobuf/src/google/protobuf/test_util.cc",
		"protobuf/src/google/protobuf/test_util_lite.cc",
		"protobuf/src/google/protobuf/*_unittest_lite.cc",
		"protobuf/src/google/protobuf/*_test_lite.cc",
		"protobuf/src/google/protobuf/*_test_util_lite.cc",
		"protobuf/src/google/protobuf/stubs/*_unittest.cc",
		"protobuf/src/google/protobuf/stubs/*_test.cc",
		"protobuf/src/google/protobuf/stubs/*_test_util.cc",
	]

	Export {
		Depends { name: "cpp" }

		cpp.includePaths: [buildDirectory, sourceDirectory, "protobuf/src", "protobuf/src/google/protobuf"]
	}

	cpp.includePaths: [buildDirectory, sourceDirectory, "protobuf/src", "protobuf/src/google/protobuf"]
	cpp.defines: qbs.targetOS.contains("windows") ? ["_WIN32"] : ["HAVE_PTHREAD"]

	Depends { name: "cpp" }
}
