{
  description = "cpp jinja interpretator with exnensions";
  inputs = {
    nixpkgs.url = "nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    ascip.url = "github:zerhud/ascip";
    ascip.inputs.nixpkgs.follows = "nixpkgs";
  };
  outputs = params@{ self, nixpkgs, flake-utils, ascip }:
    flake-utils.lib.eachDefaultSystem (system:
    let
      #lib = nixpkgs.lib;
      pkgs = import nixpkgs {
        inherit system;
        config.allowUnfreePredicate = pkg: builtins.elem (nixpkgs.lib.getName pkg) [ "clion" ];
      };

      #ascip = import params.ascip { inherit system; };
      der = pkgs.gcc13Stdenv.mkDerivation {
        name = "bastard";
        nativeBuildInputs = [pkgs.clang_15 ascip.packages."${system}".default pkgs.jetbrains.clion];
        installPhase = "mkdir -p \"$out/include\" && cp ascip.hpp -t \"$out/include\"";
        buildPhase = "g++ -std=c++23 -fwhole-program -march=native ./test.cpp -o ascip_test && ./ascip_test";
        meta.description = "bastard is an jinja interpretator with extensions writted in cpp. it can to be embbadded in your projects with open source license.";
        src = ./.;
      };
    in rec {
      devShell = der;
      packages.default = der;
      packages.bastard = der;
      defaultPackage = der;
    });
}
