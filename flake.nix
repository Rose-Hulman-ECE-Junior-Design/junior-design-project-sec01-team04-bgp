{
  description = "ECE362 Junior Design Project";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    arduino-nix.url = "github:bouk/arduino-nix";
    arduino-index = {
      url = "github:bouk/arduino-indexes";
      flake = false;
    };
  };

  outputs = {
    nixpkgs,
    flake-utils,
    arduino-nix,
    arduino-index,
    ...
  }:
    (flake-utils.lib.eachDefaultSystem (system:
      let
        # overlays = [
        #   (arduino-nix.overlay)
        #   (arduino-nix.mkArduinoPackageOverlay (arduino-index + "/index/package_index.json"))
        #   (arduino-nix.mkArduinoPackageOverlay (arduino-index + "/index/package_esp32_index.json"))
        #   (arduino-nix.mkArduinoLibraryOverlay (arduino-index + "/index/library_index.json"))
        # ];
        pkgs = (import nixpkgs) { inherit system overlays; };
        # packages.arduino-cli = pkgs.wrapArduinoCLI {
        #   libraries = with pkgs.arduinoLibraries; [
        #     (arduino-nix.latestVersion ADS1X15)
        #     (arduino-nix.latestVersion Ethernet_Generic)
        #     (arduino-nix.latestVersion SCL3300)
        #     (arduino-nix.latestVersion TMCStepper)
        #     (arduino-nix.latestVersion pkgs.arduinoLibraries."Adafruit PWM Servo Driver Library")
        #   ];
        #
        #   packages = with pkgs.arduinoPackages; [
        #     platforms.arduino.avr."1.6.23"
        #     platforms.rp2040.rp2040."2.3.3"
        #   ];
        # };
      in {
        # packages.default = 

        formatter = pkgs.nixpkgs-fmt;

        devShells.default = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            minify
            unixtools.xxd
            protobuf_27
            arduino_cli
          ];
        };
      };
    ));
}
