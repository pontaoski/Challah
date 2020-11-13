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
		name: "Translation files"
		files: ["po/*.ts"]
		fileTags: ["murmur-ts"]
		Qt.core.resourcePrefix: "/po"
	}

	Rule {
		multiplex: true
		inputs: ["murmur-qm"]
		Artifact {
			filePath: product.Qt.core.resourceFileBaseName + ".qrc"
			fileTags: ["qrc"]
		}
		prepare: {
			var cmd = new JavaScriptCommand();
			cmd.description = "generating " + output.fileName;
			cmd.sourceCode = function() {
				var doc = new Xml.DomDocument("RCC");

				var rccNode = doc.createElement("RCC");
				rccNode.setAttribute("version", "1.0");
				doc.appendChild(rccNode);

				var inputsByPrefix = {}
				for (var i = 0; i < inputs["murmur-qm"].length; ++i) {
					var inp = inputs["murmur-qm"][i];
					var prefix = "/po";
					var inputsList = inputsByPrefix[prefix] || [];
					inputsList.push(inp);
					inputsByPrefix[prefix] = inputsList;
				}

				for (var prefix in inputsByPrefix) {
					var qresourceNode = doc.createElement("qresource");
					qresourceNode.setAttribute("prefix", prefix);
					rccNode.appendChild(qresourceNode);

					for (var i = 0; i < inputsByPrefix[prefix].length; ++i) {
						var inp = inputsByPrefix[prefix][i];
						var fullResPath = inp.filePath;
						var baseDir = inp.Qt.core.resourceSourceBase;
						var resAlias = baseDir
							? FileInfo.relativePath(baseDir, fullResPath) : inp.fileName;

						var fileNode = doc.createElement("file");
						fileNode.setAttribute("alias", resAlias);
						qresourceNode.appendChild(fileNode);

						var fileTextNode = doc.createTextNode(fullResPath);
						fileNode.appendChild(fileTextNode);
					}
				}

				doc.save(output.filePath, 4);
			};
			return [cmd];
		}
	}

	Rule {
		id: tsRules

		inputs: ["murmur-ts"]
		multiplex: Qt.core.lreleaseMultiplexMode

		Artifact {
			filePath: FileInfo.joinPaths(product.Qt.core.qmDir,
					(product.Qt.core.lreleaseMultiplexMode
						? product.Qt.core.qmBaseName
						: input.baseName) + ".qm")
			fileTags: ["murmur-qm"]
		}

		prepare: {
			var inputFilePaths;
			if (product.Qt.core.lreleaseMultiplexMode)
				inputFilePaths = inputs["ts"].map(function(artifact) { return artifact.filePath; });
			else
				inputFilePaths = [input.filePath];
			var args = ['-silent', '-qm', output.filePath].concat(inputFilePaths);
			var cmd = new Command(product.Qt.core.binPath + '/'
									+ product.Qt.core.lreleaseName, args);
			cmd.description = 'Creating ' + output.fileName;
			cmd.highlight = 'filegen';
			return cmd;
		}
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
