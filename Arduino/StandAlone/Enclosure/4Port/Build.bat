@echo off

@echo Deleting old STL files.
#del *.stl

@echo Building 4 Port USB Switch case

@echo Base box
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-4Port-Base.stl -D "buildBase=true;includeDisplay=false;buildLid=false;" UsbSwitchCase.scad

echo Lids
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-4Port-Lid-Basic.stl -D "buildBase=false;includeDisplay=false;buildLid=true;includeBattery=false;includeFeet=false" UsbSwitchCase.scad
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-4Port-Lid-Battery-NoFeet.stl -D "buildBase=false;includeDisplay=false;buildLid=true;includeBattery=true;includeFeet=false" UsbSwitchCase.scad
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-4Port-Lid-Battery-WithFeet.stl -D "buildBase=false;includeDisplay=false;buildLid=true;includeBattery=true;includeFeet=true" UsbSwitchCase.scad

