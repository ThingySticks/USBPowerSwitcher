
$fn=60;

includeTerminalBlock = false;
includeArduinoUsb = false;
includeUsbMicro = false;

includeText = true;

pcbThickness = 1.2;

pcbPadding = 4;
width = 55+(2*pcbPadding);
height = 106+(2*2);
depth = 35;

pcbXOffset = 4.5;
pcbYOffset = 2.5;
pcbZOffset = 12; // 11 worked well, but +1 for extra display space.
baseThickness = 2;


module pcbUnderMounted() {

    cube([55, 99.7, pcbThickness]);
    
    // Arduino
    translate([14.7, 36.1, pcbThickness]) {
        // 64mm... long.
        //cube([40-14.7, 104.8-36.1, 22]);
    }
    
    translate([14.7, 42.8, pcbThickness]) {
        // 64mm... long.
        cube([40-14.7, 62, 14]);
        
        // Headers.
        translate([0, 7.5, pcbThickness]) {
            cube([40-14.7, 45, 22.5]);
        }
        
        
    }
    
    // Arduino USB
    translate([14.7 + (25.3 - 10)/2, 104.8, pcbThickness + 13.3]) {
        cube([10, 15, 3.5]);
    }
    
usbWidth = 13.32;
usbHeight = 6.8;
    
    // USB 1
    translate([16.5-6.66, 31-5, -13.3]) {
        cube([usbWidth, usbHeight, 13.3]);
    }
    
    // USB 2
    translate([16.5+22-6.66, 31-5, -13.3]) {
        cube([usbWidth, usbHeight, 13.3]);
    }
   
    // USB 3
    translate([16.5-6.66, 14.25-5, -13.3]) {
        cube([usbWidth, usbHeight, 13.3]);
    }
    
    // USB 4
    translate([16.5+22-6.66, 14.25-5, -13.3]) {
        cube([usbWidth, usbHeight, 13.3]);
    }
    
    // LEDs
    translate([50, 49.9, -12.65]) {
            
        // LED1
        translate([0, 30, 0]) {
            cylinder(d=4, h=12.65);
        }
        
        // LED2
        translate([0, 20, 0]) {
            cylinder(d=4, h=12.65);
        }
        
        // LED3
        translate([0, 10, 0]) {
            cylinder(d=4, h=12.65);
        }
        
        // LED4
        translate([0, 0, 0]) {
            cylinder(d=4, h=12.65);
        }
    }
    
    // Switches
    translate([5, 49.9, -12.65]) {
    
        // Switch 1
        translate([0, 30, 0]) {
            cylinder(d=4, h=12.65);
        }
        
        // Switch 2
        translate([0, 20, 0]) {
            cylinder(d=4, h=12.65);
        }
        
        // Switch 3
        translate([0, 10, 0]) {
            cylinder(d=4, h=12.65);
        }
        
        // Switch 4
        translate([0, 00, 0]) {
            cylinder(d=4, h=12.65);
        }
    }
    
    // 5V terminal
    // On the underside of the PCB.
    translate([21.8, 91, -9]) {
        cube([11.1, 7.5, 9]);
        
        // Push through the case.
        cube([11.1, 27.5, 9]);
    }
    
    // USB B
    translate([24, -1, pcbThickness]) {
        #cube([30.9-24, 10, 4]);
    }
    
    // Screws holes.
    translate([4, 12, -20]) {
        cylinder(d=4, h=40);
    }
    
    translate([4+47, 12, -20]) {
        cylinder(d=4, h=40);
    }
    
    translate([4, 12+81, -20]) {
        cylinder(d=4, h=40);
    }
    
    translate([4+47, 12+81, -20]) {
        cylinder(d=4, h=40);
    }
}

module tftBob() {
    
    // Display 33mm wide (+/- ears of 5.5mm)
    // Display height 36mm (+6, -2.5ish)
    
    // mounts
    tftBobMount(-2.6,2.3);
    tftBobMount(33+2.6,2.3);
    
    cube([33,45,1.6]);
    translate([0, 6, -3]) {
        
        cube([33, 36, 3]);
    }
    
    // Pins
    translate([0, 6, 1.6]) {
        cube([33, 4, 7]);
    }
    
    // mounts
    tftBobMount(-2.6, 42);
    tftBobMount(33+2.6,42);
}

module tftBobMount(x,y) {
    translate([x,y,-10]) {
        cylinder(d=2.3, h=20);
    }
    translate([x,y,0]) {
        cylinder(d=5.5, h=1.6);
    }
}


module noCornersCube(width, height, depth, cornerRadius) {
    
    //cube([width, height, depth]);
    union() {
        linear_extrude(height=depth) {
            // Square with corners cut.
            polygon([
            [cornerRadius,0],
            [width-cornerRadius, 0],
            [width, cornerRadius],
            [width, height-cornerRadius],
            [width-cornerRadius, height],
            [cornerRadius, height],
            [0, height-cornerRadius],
            [0, cornerRadius],
            [cornerRadius,0]
            ]);
        }
        
    }
}

module roundedCube(width, height, depth, cornerRadius) {
    
    //cube([width, height, depth]);
    union() {
        translate([cornerRadius,cornerRadius,0]) {
            cylinder(r=cornerRadius, h=depth);   
        }
        translate([width-cornerRadius,cornerRadius,0]) {
            cylinder(r=cornerRadius, h=depth);
        }
        translate([cornerRadius,height-cornerRadius,0]) {
            cylinder(r=cornerRadius, h=depth);
        }
        translate([width-cornerRadius,height-cornerRadius,0]) {
            cylinder(r=cornerRadius, h=depth);
        }
        
        noCornersCube(width, height, depth, cornerRadius);
    }
}



module pcbHole(x,y) {
    translate([x,y,0]) {
        // 4.2mm for a M3 heat fit 
        #cylinder(d=4.2, h=20);
    }
}

module pcbHoles() {
    translate([pcbXOffset, pcbYOffset,5]) {
            // Screws holes.
        pcbHole(4, 12); 
        pcbHole(4+47, 12);
        pcbHole(4, 12+81);
        pcbHole(4+47, 12+81);
    }
}

module pcbMount(x,y, h) {
    translate([x,y,0]) {
        cylinder(d=8, h=h);
    }
}

module pcbMounts() {
    translate([pcbXOffset, pcbYOffset,0]) {
            // Screws holes.
        pcbMount(4, 12, pcbZOffset); 
        pcbMount(4+47, 12, pcbZOffset);
        pcbMount(4, 12+81, pcbZOffset);
        pcbMount(4+47, 12+81, pcbZOffset);
    }
}

module switchCutouts() {
    
switchDiameter = 4;
switchHeight = 12.65;
    
    translate([pcbXOffset, pcbYOffset,0]) {
               
        translate([5, 49.9, -0.01]) {    
            // Switch 1
            translate([0, 30, 0]) {
                cylinder(d=switchDiameter, h=switchHeight);
            }
            
            // Switch 2
            translate([0, 20, 0]) {
                cylinder(d=switchDiameter, h=switchHeight);
            }
            
            // Switch 3
            translate([0, 10, 0]) {
                cylinder(d=switchDiameter, h=switchHeight);
            }
            
            // Switch 4
            translate([0, 00, 0]) {
                cylinder(d=switchDiameter, h=switchHeight);
            }
        }
    }
}
    
module ledCutouts(){
    
ledDiameter = 3.6;
ledHeight = 12.65;
    translate([pcbXOffset, pcbYOffset,0]) {
       
        translate([50, 49.9, -0.01]) {
            
            // LED1
            translate([0, 30, 0]) {
                cylinder(d=ledDiameter, h=ledHeight );
            }
            
            // LED2
            translate([0, 20, 0]) {
                cylinder(d=ledDiameter, h=ledHeight );
            }
            
            // LED3
            translate([0, 10, 0]) {
                cylinder(d=ledDiameter, h=ledHeight );
            }
            
            // LED4
            translate([0, 0, 0]) {
                cylinder(d=ledDiameter, h=ledHeight );
            }
        }
    }
}

module usbOutletCutouts(){
    
usbWidth = 13.32+2;
usbHeight = 6.8+2;
    
    translate([pcbXOffset, pcbYOffset - 1.5,0]) {
        
        // USB 1
        translate([16.5-6.66-1, 31-5, -0.01]) {
            cube([usbWidth, usbHeight, 4]);
        }
        
        // USB 2
        translate([16.5+22-6.66-1, 31-5, -0.01]) {
            cube([usbWidth, usbHeight, 4]);
        }
       
        // USB 3
        translate([16.5-6.66-1, 14.25-5, -0.01]) {
            cube([usbWidth, usbHeight, 4]);
        }
        
        // USB 4
        translate([16.5+22-6.66-1, 14.25-5, -0.01]) {
            cube([usbWidth, usbHeight, 4]);
        }
    }        
}
module topArduinoUsbCutout(){
    translate([pcbXOffset, pcbYOffset, pcbZOffset]) {
        translate([14.7 + (25.3 - 10)/2-1, 102, pcbThickness + 13.3-1.5]) {
            #cube([10+2, 15, 3+4]);
        }
        
        //translate([14.7 + (25.3 - 10)/2, 104.8, pcbThickness + 13.3]) {
        //    cube([10, 15, 3.5]);
        //}
    }
}

module TerminalBlockConnectorCutout() {
    translate([pcbXOffset, pcbYOffset, pcbZOffset]) {
        translate([21.0, 91, -9]) {
        cube([12.8, 27.5, 9]);
        }
    }
}
    
// Difficult to solder connector!
module bottomUsbCutout() {
    translate([pcbXOffset, 0, pcbZOffset + pcbThickness - 3]) {
        translate([24-2.5, -0.1, pcbThickness-1]) {
            cube([12, 10, 8]);
        }
    }
}

//translate([(width-33)/2,height-63,-10]) {
    
tftBobXOffset = (width-33)/2;
tftBobYOffset = height-63;

module tftBobCutout() {
    
    // Display 33mm wide (+/- ears of 5.5mm)
    // Display height 36mm (+6, -2.5ish)
    
    // Don't cut all the way through with the main display, allow 0.4mm
    // i.e. 2 layers, for a bezzle
    translate([tftBobXOffset-0.6, tftBobYOffset-0.6, 0.4]) {
        // base
        //cube([33,45,1.6]);
        
        // display
        translate([0, 6, 0]) {
            
            cube([33+1.2, 36+1.2, baseThickness+1]);
        }
            
        translate([1, 1.5, 0.5]) {
            // Small recess for pins.
            #cube([31.5, 3, baseThickness+1]);
        }
        
        // Cut the final inner display out.
        // 3mm bezel on x.
        translate([3.5, 6 +1.5, -0.5]) {
            
            // SD card faces same direction as Arduino.
            #cube([33+1.2-6.5, 36+1.2 - (6+1.5), baseThickness+1]);
        }
    }
}

module tftBobPins() {
    translate([tftBobXOffset, tftBobYOffset, 0]) {
        tftBobPin(-2.6,2.3);
        tftBobPin(33+2.6,2.3);
        
        
        // mounts
        tftBobPin(-2.6, 42);
        tftBobPin(33+2.6,42);
    }
}

module tftBobPin(x,y) {
    translate([x,y,0]) {
        // 2.0mm pins is just tight.
        cylinder(d=2.0, h=4+baseThickness);
        
        // Raise it up 1.4mm to allow for solder joint of pins 
        // and for unevenness in display.
        // watch as upper pins likely to clash with display cutout
        cylinder(d=4.4, h=baseThickness + 1.4);
    }
}


module lidMount(x,y, offsetx, offsety) {
    
    translate([x,y,1]) {
        hull() {
            translate([offsetx,offsety,7]) {
                cylinder(d=1, h=2);
            }
            translate([0,0,depth-13]) {
                cylinder(d=8, h=2);
            }
        }
    }
    
    translate([x,y,depth-12]) {
        difference() {
            union() {
                // -1.5 for lid inner thickness.
                cylinder(d=8, h=(12-4-1));
            }
            union() {
                translate([0,0,-1]) {
                    cylinder(d=4.4, h=depth);
                }
            }
        }
    }
}

lidScrewOffset = 5;

module lidMounts() {
offset = lidScrewOffset;

    lidMount(offset ,offset ,-2.5,-2.5);
    lidMount(offset ,height-offset , -2.5, 2.5);
    lidMount(width-offset ,height-offset, 2.5,2.5);
    lidMount(width-offset ,offset,2.5,-2.5 );
}

module bodyMain() {
    difference() {
        union() {
            roundedCube(width, height, depth, 6);
        }
        union() {
            translate([1.5, 1.5, 2]) {
                roundedCube(width-3, height-3, (depth-2)+0.01, 6-baseThickness);
            }
            
            pcbHoles();
            
            switchCutouts();
            ledCutouts();
            usbOutletCutouts();

            if (includeArduinoUsb) {
                topArduinoUsbCutout();
            }

            if (includeTerminalBlock) {
                TerminalBlockConnectorCutout();
            }
            
            if (includeUsbMicro) {
                bottomUsbCutout();
            }
        }
    }
}

module usbPortTextCutout(x,y, text, size) {
    translate([x, y, 0]) {
        // Text is back to front!
            rotate([0,180,0]) {
                linear_extrude(1)  {
                    text(text, size=size);
                }
            }
        }
}

module textCutout() {
textReccess = 0.5; // 1-2 layer heights
    
    // USB ports.
    translate([16.5, 1, textReccess]) {
        usbPortTextCutout(-4, 31-5, "2", 8);
        usbPortTextCutout(42, 31-5, "1", 8);
        usbPortTextCutout(-4, 14.25-5, "4", 8);
        usbPortTextCutout(42, 14.25-5, "3", 8);
    }
    
    // Buttons/LEDs....
    translate([width/2 + 9, 50, textReccess]) {
        usbPortTextCutout(0, 0, "Port 4", 5);
        usbPortTextCutout(0, 10, "Port 3", 5);
        usbPortTextCutout(0, 20, "Port 2", 5);
        usbPortTextCutout(0, 30, "Port 1", 5);
    }
    
    translate([width-3, 65, textReccess]) {
        usbPortTextCutout(0, 30, "USB Power Switch", 5);
    }
}


module body() {
    difference() {
        union() {
            bodyMain();
            pcbMounts();
            lidMounts();
        }
        union() {
            pcbHoles();
            if (includeText) {
                textCutout();
            }
            
        }
    }
}

    


translate([pcbXOffset, pcbYOffset,pcbZOffset]) {

    //%pcb();
    %pcbUnderMounted();
}

// display 33mm wide.
    
// Display 33mm wide (+/- ears of 5.5mm)
// Display height 36mm (+6, -2.5ish)
    
translate([(width-33)/2,height-63,baseThickness + 0.8]) {
    //%tftBob();
}

body();