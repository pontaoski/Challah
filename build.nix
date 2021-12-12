{ rev
, stdenv
, lib
, wrapQtAppsHook
, makeWrapper
, protobuf3_12
, libsForQt5
, gst_all_1
, qbs
, cmake
, pkg-config
, json-glib
, protocol
}: stdenv.mkDerivation {
  pname = "challah";
  version = builtins.substring 0 8 rev;

  buildInputs =
    [ protobuf3_12 json-glib protocol ]
    ++ (with gst_all_1; [ gst-plugins-bad gst-plugins-base gstreamer ])
    ++ (with libsForQt5; [ full kirigami2 ]);
  nativeBuildInputs = [ wrapQtAppsHook makeWrapper qbs cmake pkg-config ];

  src = builtins.fetchGit {
    inherit rev;
    url = "https://github.com/harmony-development/Challah.git";
    submodules = true;
    allRefs = true;
  };

  preConfigure = ''
    export HOME=/tmp/challah-tmp-build-home/
    mkdir -p $HOME
  '';

  configurePhase = ''
    runHook preConfigure
    
    qbs setup-toolchains --detect
    qbs resolve profile:gcc
  '';

  buildPhase = ''
    qbs build
  '';

  installPhase = ''
    mkdir -p $out
    qbs install --install-root $out
    mv $out/usr/local/* $out
    rm -r $out/usr $out/bin/Challah.debug
  '';
}
