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
    devshell.url = "github:numtide/devshell";
  };

  outputs = inputs:
    with inputs.flakeUtils.lib; eachSystem [ "x86_64-linux" ] (system:
      let
        pkgs = import inputs.nixpkgs {
          inherit system;
          overlays = [ inputs.devshell.overlay ];
        };
        devShell = pkgs.devshell.fromTOML ./devshell.toml;
        packages = {
          challah = pkgs.libsForQt5.callPackage ./build.nix {
            inherit devShell;
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
        inherit devShell packages apps;

        defaultPackage = packages.challah;
        defaultApp = apps.challah;
      }
    );
}
