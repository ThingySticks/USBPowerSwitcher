@echo off

@echo Deleting old STL files.
del *.stl

@echo Building 4 Port USB Switch case

@echo Base box
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-4Port-Base-All-NoText.stl -D "includeTerminalBlock=true;includeArduinoUsb=true;includeUsbMicro=true;includeText=false" UsbSwitchCase-4Port.scad
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-4Port-Base-TerminalBlock-NoText.stl -D "includeTerminalBlock=true;includeArduinoUsb=false;includeUsbMicro=false;includeText=false" UsbSwitchCase-4Port.scad
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-4Port-Base-Arduino-NoText.stl -D "includeTerminalBlock=false;includeArduinoUsb=true;includeUsbMicro=false;includeText=false" UsbSwitchCase-4Port.scad
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-4Port-Base-UsbMicro-NoText.stl -D "includeTerminalBlock=false;includeArduinoUsb=false;includeUsbMicro=true;includeText=false" UsbSwitchCase-4Port.scad

"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-4Port-Base-All-Text.stl -D "includeTerminalBlock=true;includeArduinoUsb=true;includeUsbMicro=true;includeText=true" UsbSwitchCase-4Port.scad
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-4Port-Base-TerminalBlock-Text.stl -D "includeTerminalBlock=true;includeArduinoUsb=false;includeUsbMicro=false;includeText=true" UsbSwitchCase-4Port.scad
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-4Port-Base-Arduino-Text.stl -D "includeTerminalBlock=false;includeArduinoUsb=true;includeUsbMicro=false;includeText=true" UsbSwitchCase-4Port.scad
"C:\Program Files\OpenSCAD\openscad.com" -o UsbSwitchCase-4Port-Base-UsbMicro-Text.stl -D "includeTerminalBlock=false;includeArduinoUsb=false;includeUsbMicro=true;includeText=true" UsbSwitchCase-4Port.scad
