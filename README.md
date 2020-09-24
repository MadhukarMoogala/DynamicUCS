# Implement Dynamic UCS for custom solids

![ObjectArx2021](https://img.shields.io/badge/ObjectARX-2021-green)
![AutoCAD2021](https://img.shields.io/badge/AutoCAD-2021-green)



### Dev Setup

*Make sure create a User Environment Variable `ObjectArxPath` and set it your SDK path*

```bash
git clone https://github.com/MadhukarMoogala/DynamicUCS
cd DynamicUCS
msbuild /t:build /p:Configuration=Debug;Platform=x64
or devenv DynamicUCS.vcxproj
```

### Run

```bash
Launch AutoCAD 2021
appload "~\DynamicUCS\x64\Debug\DynamicUCS.arx"
MyBox
10
10
10
Circle
hover on any face of Box.
```

### Demo

![DynmicsUCS.gif](https://github.com/MadhukarMoogala/DynamicUCS/blob/master/DynamicUCS.gif)

## License

This sample is licensed under the terms of the [MIT License](http://opensource.org/licenses/MIT). Please see the [LICENSE](https://github.com/MadhukarMoogala/design-migration/blob/master/LICENSE) file for full details.

## Written by

Madhukar Moogala, [Forge Partner Development](http://forge.autodesk.com/) @galakar



