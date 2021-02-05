Product {
	name: "vendored_protobuf"

	type: ["staticlibrary"]
	Group {
		files: ["protobuf/cmake/CMakeLists.txt"]
		fileTags: ["cmake_project"]
	}
	Group {
		files: ["protobuf/src/*"]
		fileTags: ["cmake_sources"]
	}
	Rule {
		inputs: ["cmake_project"]
		auxiliaryInputs: ["cmake_sources"]

		Artifact { filePath: product.sourceDirectory+"/protobuf/cmake/libprotobuf.a"; fileTags: ["staticlibrary"] }

		prepare: {
			var args = [
				"-DCMAKE_BUILD_TYPE=Release",
				"-Dprotobuf_BUILD_TESTS=OFF",
				"-Dprotobuf_BUILD_PROTOC_BINARIES=OFF",
				".",
			]

			var config = new Command("cmake", args);
			config.description = "configuring protobuf...";
			config.workingDirectory = product.sourceDirectory + "/protobuf/cmake"

			var build = new Command("cmake", ["--build", ".", "-j10"]);
			build.description = "building protobuf...";
			build.workingDirectory = product.sourceDirectory + "/protobuf/cmake"

			return [config, build];
		}
	}
}
