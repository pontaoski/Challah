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
    rev = "26c057dbb77eb7349e5f645bb3d62eb0d25086cb";
    sha256 = "sha256-4s7s0LXIitkVym3S9WqvHXQhm9CtBNufBKKa8TwZAMU=";
  };

  configurePhase = ''
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX="$out" ..
  '';

  installPhase = ''
    mkdir -p $out/include
    make install
  '';
}
