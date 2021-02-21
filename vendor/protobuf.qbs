import qbs.File

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

		Artifact { filePath: product.buildDirectory+"/libprotobuf.a"; fileTags: ["staticlibrary"] }

		prepare: {
			var args = [
				"-DCMAKE_BUILD_TYPE=Release",
				"-Dprotobuf_BUILD_TESTS=OFF",
				"-Dprotobuf_BUILD_PROTOC_BINARIES=OFF",
				product.sourceDirectory + "/protobuf/cmake",
			]

			File.makePath(product.buildDirectory)

			var config = new Command("cmake", args);
			config.description = "configuring protobuf...";
			config.workingDirectory = product.buildDirectory

			var build = new Command("cmake", ["--build", ".", "-j4"]);
			build.description = "building protobuf...";
			build.workingDirectory = product.buildDirectory

			return [config, build];
		}
	}
}
