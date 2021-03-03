# Challah

The friendly chat application.

## Build Instructions

If you don't know what you're doing:

Get protobuf/protoc from your distro, as well as Qt packages.

```
git clone --recurse-submodules https://github.com/harmony-development/Challah
cd Challah
qbs -j4 project.vendoredQQC2BreezeStyle:true project.vendoredKirigami:true
```

If you do know what you're doing and want to develop against system dependencies.
Probably requires git master KDE software.

```
qbs -j4
```
