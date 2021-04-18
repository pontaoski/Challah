{ rev
, devShell
, stdenv
, wrapQtAppsHook
, makeWrapper
, protobuf3_12
, libsForQt5
, qbs
, cmake
}: stdenv.mkDerivation {
  pname = "challah";
  version = builtins.substring 0 8 rev;

  buildInputs = [ protobuf3_12 ] ++ (with libsForQt5; [ full kirigami2 ]);
  nativeBuildInputs = [ wrapQtAppsHook makeWrapper qbs cmake ];

  src = builtins.fetchGit {
    inherit rev;
    url = "https://github.com/harmony-development/Challah.git";
    submodules = true;
    allRefs = true;
  };

  configurePhase = ''
    source ${devShell}
  '';

  buildPhase = ''
    qbs build
  '';

  installPhase = ''
    mkdir -p $out
    qbs install --install-root $out
    mv $out/usr/local/* $out
    rm -r $out/usr $out/bin/Challah.debug

    wrapProgram $out/bin/Challah --set LD_LIBRARY_PATH $LD_LIBRARY_PATH
  '';
}
