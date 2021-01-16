FROM kdeorg/android-sdk:latest

ARG QT_TAG=v5.15.2

# QBS

RUN cd && git clone git://code.qt.io/qt/qtbase.git --single-branch --branch ${QT_TAG} && cd qtbase \
	&& ./configure -prefix /opt/nativetooling -opensource -confirm-license -no-gui -release -optimize-size -nomake tests -nomake examples \
	&& make -j`nproc` && make install && rm -rf ~/qtbase

RUN /opt/helpers/build-cmake-native \
	qbs https://github.com/qbs/qbs.git \
	-DCMAKE_INSTALL_PREFIX=/opt/nativetooling \
	-DQBS_INSTALL_PREFIX=/opt/nativetooling \
	-DCMAKE_INSTALL_PREFIX=/opt/nativetooling \
	-DBUILD_TESTING=OFF \
	-DQt5_DIR=/opt/nativetooling/lib/cmake/Qt5 \
	-DKCONFIG_USE_GUI=OFF \
	-DQt5Core_DIR=/opt/nativetooling/lib/cmake/Qt5Core \
	-DQt5Qml_DIR=/opt/nativetooling/lib/cmake/Qt5Qml \
	-DQt5Concurrent_DIR=/opt/nativetooling/lib/cmake/Qt5Concurrent \
	-DCMAKE_DISABLE_FIND_PACKAGE_Qt5Widgets=ON

# gRPC

RUN git clone --depth=1 --recursive https://github.com/grpc/grpc.git

RUN /opt/helpers/build-cmake-native \
	grpc https://github.com/grpc/grpc.git \
	-DCMAKE_INSTALL_PREFIX=/opt/nativetooling \
	-DBUILD_SHARED_LIBS=on \
	-DCARES_SHARED=on \
	-DCARES_STATIC=off \
	-DgRPC_BUILD_CSHARP_EXT=OFF \
	-DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF \
	-DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF \
	-DgRPC_BUILD_GRPC_OBJECTIVE_C_PL=OFF \
	-DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
	-DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF \
	-Dprotobuf_BUILD_SHARED_LIBS=on \
	-DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF \
	-DRE2_BUILD_TESTING=off \
	-DgRPC_BUILD_TESTS=off \
	-DgRPC_SSL_PROVIDER=package \
	-DgRPC_ZLIB_PROVIDER=package

RUN /opt/helpers/build-cmake \
	grpc https://github.com/grpc/grpc.git \
	-DCMAKE_INSTALL_PREFIX=/opt/nativetooling \
	-DBUILD_SHARED_LIBS=on \
	-DCARES_SHARED=on \
	-DCARES_STATIC=off \
	-DgRPC_BUILD_CSHARP_EXT=OFF \
	-DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF \
	-DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF \
	-DgRPC_BUILD_GRPC_OBJECTIVE_C_PL=OFF \
	-DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
	-DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF \
	-Dprotobuf_BUILD_SHARED_LIBS=on \
	-DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF \
	-DRE2_BUILD_TESTING=off \
	-DgRPC_BUILD_TESTS=off \
	-DgRPC_SSL_PROVIDER=package \
	-DgRPC_ZLIB_PROVIDER=package

RUN /opt/helpers/build-cmake \
	kirigami https://invent.kde.org/frameworks/kirigami.git
