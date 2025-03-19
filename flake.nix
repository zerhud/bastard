{
  description = "cpp jinja interpretator with exnensions";
  inputs = {
    nixpkgs.url = "nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    ascip.url = "github:zerhud/ascip";
    ascip.inputs = {
      nixpkgs.follows = "nixpkgs";
      cppbm.follows = "cppbm";
    };
    cppbm.url = "github:zerhud/cppbm";
    cppbm.inputs.nixpkgs.follows = "nixpkgs";
    nixpkgs_jetbrains.url = "github:nixos/nixpkgs/06cf0e1da4208d3766d898b7fdab6513366d45b9";
  };
  outputs = params@{ self, nixpkgs, ... }:
    params.flake-utils.lib.eachDefaultSystem (system:
    let
      #lib = nixpkgs.lib;
      ascip = params.ascip.packages."${system}".default;
      tref = params.cppbm.packages."${system}".tref;
      pkgs = import nixpkgs {
        inherit system;
        config.allowUnfreePredicate = pkg: builtins.elem (nixpkgs.lib.getName pkg) [ "clion" ];
      };
      pkgs_jb = import params.nixpkgs_jetbrains {
        inherit system;
        config.allowUnfreePredicate = pkg: builtins.elem (nixpkgs.lib.getName pkg) [ "clion" ];
      };

      snitch = pkgs.gcc14Stdenv.mkDerivation {
        name = "snitch_tests";
        buildInputs = [ ];
        nativeBuildInputs = with pkgs; [ cmake ninja python3 ];
        #cmakeFlags = [ "-D" "snitch:create_library=true" ];
        #cmakeFlags = [ "-DSNITCH_HEADER_ONLY=ON" ];
        src = pkgs.fetchzip {
          url = "https://github.com/cschreib/snitch/archive/refs/tags/v1.2.5.tar.gz";
          sha256 = "sha256-7O3L9v/TXKfbAl/G1MKKluLgj6v5m1x4C+/+DMLRyhM=";
        };
      };

      gcc_with_debinfo = pkgs.gcc14.cc.overrideAttrs(oldAttrs: { dontStrip=true; });
      stdenv_with_debinfo = pkgs.overrideCC pkgs.gcc14Stdenv gcc_with_debinfo;
      der = pkgs.gcc14Stdenv.mkDerivation {
        name = "jiexpr";
        NIX_ENFORCE_NO_NATIVE = false;
        buildInputs = with pkgs;[ tref ];
        snitch_header = snitch.out;
        nativeBuildInputs = with pkgs;[gdb clang_20 ninja ascip snitch boost186 (boost-build.override {useBoost = boost186;})];
        installPhase = "mkdir -p \"$out/include\" && cp ascip.hpp -t \"$out/include\"";
        buildPhase = "g++ -std=c++23 -fwhole-program -march=native ./test.cpp -o ascip_test && ./ascip_test";
        meta.description = "jiexpr is an jinja interpretator with extensions writted in cpp. it can to be embbadded in your projects with open source license.";
        src = ./.;
        CPATH = pkgs.lib.strings.concatStringsSep ":" [
          "${snitch}/include/snitch"
        ];
        LIBRARY_PATH = pkgs.lib.strings.concatStringsSep ":" [
          "${snitch}/lib"
        ];
      };
    in rec {
      #TODO: unset SOURCE_DATE_EPOCH for devShell (for provide random value in compile time)
      devShell = der.overrideAttrs(finalAttrs: previousAttrs: {
          nativeBuildInputs =  previousAttrs.nativeBuildInputs ++ [ pkgs.jetbrains.clion ];
        });
      packages.default = der;
      packages.jiexpr = der;
      defaultPackage = der;
    });
}
