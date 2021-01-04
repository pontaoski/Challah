Product {
	type: ["dynamiclibrary"]
	Group {
		files: ["grpc/CMakeLists.txt"]
		fileTags: ["cmake_project"]
	}
	Group {
		files: ["grpc/*"]
		excludeFiles: ["grpc/CMakeLists.txt"]
		fileTags: ["cmake_sources"]
	}
	Rule {
		inputs: ["cmake_project"]
		auxiliaryInputs: ["cmake_sources"]

		Artifact { filePath: "grpc/libgrpc.a"; fileTags: ["staticlibrary"] }
		Artifact { filePath: "grpc/third_party/zlib/libz.a"; fileTags: ["staticlibrary"] }
		Artifact { filePath: "grpc/third_party/cares/cares/lib/libcares.a"; fileTags: ["staticlibrary"] }
		Artifact { filePath: "grpc/third_party/boringssl/crypto/libcrypto.a"; fileTags: ["staticlibrary"] }
		Artifact { filePath: "grpc/third_party/boringssl/ssl/libssl.a"; fileTags: ["staticlibrary"] }
		Artifact { filePath: "grpc/third_party/protobuf/libprotobuf.a"; fileTags: ["staticlibrary"] }
		Artifact { filePath: "grpc/libgpr.a"; fileTags: ["staticlibrary"] }
		Artifact { filePath: "grpc/libaddress_sorting.a"; fileTags: ["staticlibrary"] }
		Artifact { filePath: "grpc/libgrpc++.a"; fileTags: ["staticlibrary"] }

		prepare: {
			var args = [
				"-DCMAKE_TOOLCHAIN_FILE="+Android.ndk.ndkDir+"/build/cmake/android.toolchain.cmake",
				"-DANDROID_ABI=armeabi-v7a",
				"-DANDROID_PLATFORM=android-26",
				"-DANDROID_STL=c++_static",
				"-DRUN_HAVE_STD_REGEX=0",
				"-DRUN_HAVE_POSIX_REGEX=0",
				"-DRUN_HAVE_STEADY_CLOCK=0",
				"-DCMAKE_BUILD_TYPE=Release",
			]

			var config = new Command("cmake", args);
			config.description = "configuring gRPC...";
			config.workingDirectory = product.sourceDirectory + "/grpc";

			var config = new Command("cmake", ["--build", ".", "--target", "grpc++"]);
			config.description = "building gRPC...";
			config.workingDirectory = product.sourceDirectory + "/grpc";

			return [cmd];
		}
	}
}
