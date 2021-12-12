{ stdenv
, fetchFromGitHub
, buildGoModule
, lib
,
}: buildGoModule {
  name = "protoc-gen-hrpc";
  version = "main";

  src = fetchFromGitHub {
    owner = "harmony-development";
    repo = "hrpc";
    rev = "60ef34bb388d2209ec4de20512c50af8c1facced";
    sha256 = "sha256-x2IVEjdttf9z+XazoDQOeY4QU7RzK9XsExpjtQhmrKY=";
  };

  subPackages = [ "protoc-gen-hrpc" ];

  vendorSha256 = "sha256-VoOBEpdkGIonW0AHO7fYsgmyAtDcSU8V/hTR0KHB7i0=";
}
