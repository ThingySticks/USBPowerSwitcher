@echo off

@echo Deleting old STL files.
del *.stl

@echo Building 2 Port USB Switch case

@echo Base box
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-2Port-Base.stl -D "buildBase=true;includeDisplay=false;buildLid=false;" UsbSwitchCase.scad
