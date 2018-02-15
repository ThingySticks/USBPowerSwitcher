@echo off

@echo Deleting old STL files.
del *.stl

echo Lids
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-Lid-NoBattery-NoFeet.stl -D "includeBattery=false;includeFeet=false" UsbSwitchCaseLid.scad
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-Lid-NoBattery-WithFeet.stl -D "includeBattery=false;includeFeet=true" UsbSwitchCaseLid.scad
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-Lid-Battery-NoFeet.stl -D "includeBattery=true;includeFeet=false" UsbSwitchCaseLid.scad
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-Lid-Battery-WithFeet.stl -D "includeBattery=true;includeFeet=true" UsbSwitchCaseLid.scad

