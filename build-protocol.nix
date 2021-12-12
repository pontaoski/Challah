{ cmake
, stdenv
, fetchFromGitHub
,
}: stdenv.mkDerivation {
  name = "harmony-protocol";
  version = "main";

  nativeBuildInputs = [ cmake ];

  src = fetchFromGitHub {
    owner = "harmony-development";
    repo = "protocol";
    rev = "f8bdbeabd428d86887963267afbf38073c9dfb49";
    sha256 = "sha256-X/Uavda3F35e4a1XLAZLNLZDfBW8C4NqlQ1ZTgY2nrY=";
  };

  configurePhase = ''
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX="$out/usr/include" ..
  '';

  installPhase = ''
    mkdir -p $out/usr/include
    make install
  '';
}
