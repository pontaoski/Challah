{
  description = "Flake for Challah";

  inputs = {
    challahSrc = {
      url = "github:harmony-development/Challah/oven";
      flake = false;
    };
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flakeUtils.url = "github:numtide/flake-utils";
    flakeCompat = {
      url = "github:edolstra/flake-compat";
      flake = false;
    };
  };

  outputs = inputs:
    with inputs.flakeUtils.lib; eachSystem [ "x86_64-linux" ] (system:
      let
        pkgs = import inputs.nixpkgs {
          inherit system;
        };
        packages = rec {
          harmony-protocol = pkgs.callPackage ./build-protocol.nix { };
          challah = pkgs.libsForQt5.callPackage ./build.nix {
            protocol = harmony-protocol;
            inherit (inputs.challahSrc) rev;
          };
        };
        apps = {
          challah = mkApp {
            name = "Challah";
            drv = packages.challah;
          };
        };
      in
      {
        inherit packages apps;

        devShell = packages.challah;
        defaultPackage = packages.challah;
        defaultApp = apps.challah;
      }
    );
}
