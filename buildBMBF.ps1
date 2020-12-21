# Builds a .zip file for loading with BMBF
& $PSScriptRoot/build.ps1

if ($?) {
    Compress-Archive -Path "./libs/arm64-v8a/libpause-confirm.so", "./libs/arm64-v8a/libcustom-types.so", "./libs/arm64-v8a/libquestui.so", "./bmbfmod.json" -DestinationPath "./pause-confirm_v0.2.0.zip" -Update
}
