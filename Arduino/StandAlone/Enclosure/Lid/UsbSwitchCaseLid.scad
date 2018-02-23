
$fn=60;

includeBattery = true;
includeFeet = false;

pcbThickness = 1.2;

pcbPadding = 4;
width = 55+(2*pcbPadding);
height = 106+(2*2);
//depth = 35; // Case depth

pcbXOffset = 4.5;
pcbYOffset = 2.5;
pcbZOffset = 12; // 11 worked well, but +1 for extra display space.
baseThickness = 2;

lidScrewOffset = 5;
lidDepth = 4;

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
    
    // USB 1
    translate([8.4, 17.9, -13.3]) {
        cube([21.6-8.4, 24.6-17.9, 13.3]);
    }
    
    // USB 2
    translate([33.4, 17.9, -13.3]) {
        cube([46.6-33.4, 24.6-17.9, 13.3]);
    }
    
    // LED1
    translate([3.77, 59.25, -22]) {
        cylinder(d=4, h=22);
    }
    
    // LED2
    translate([3.77, 64.96, -22]) {
        cylinder(d=4, h=22);
    }
    
    // Switch 1
    translate([4.41, 82.55, -12.65]) {
        cylinder(d=4, h=12.65);
    }
    
    // Switch 2
    translate([4.41, 72.85, -12.65]) {
        cylinder(d=4, h=12.65);
    }
    
    // 5V terminal
    translate([47.8, 36.2, pcbThickness]) {
        cube([55.1-47.8, 47.5-36.2, 12]);
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


module lidHole(x,y) {
    translate([x,y,-0.1]) {
        cylinder(d=3.5, h=lidDepth+20);
        cylinder(d1=6.5, d2=3.5, h=3);
    }
}

module lidHoles() {
    lidHole(lidScrewOffset,lidScrewOffset);
    lidHole(lidScrewOffset,height-lidScrewOffset);
    lidHole(width-lidScrewOffset,height-lidScrewOffset);
    lidHole(width-lidScrewOffset,lidScrewOffset);
}

module addMountingLug(x,y) {
    translate([x,y,0]) {
        difference() {
            cylinder(d=16, h=4);
            cylinder(d1=5, d2=9, h=4.1);
        }
    }
}

module addMountingLugs() {
    addMountingLug(-5,20);
    addMountingLug(-5,height-20);
    addMountingLug(width+5,20);
    addMountingLug(width+5,height-20);
}

module lid() {
    
lidLipDepth = 4;
     difference() {
        union() {
            // Main lid...
            roundedCube(width, height, lidDepth, 6);
            
            translate([1.4, 1.4, lidDepth]) {
                // 1.5mm Goes in the box...
                roundedCube(width-2.8, height-2.8, lidLipDepth, 6);
            }
            
            if (includeFeet) {
                // Mounting Lugs
                addMountingLugs();
            }
        }
        union() {
            translate([5, 5, 1.5]) {
                // Inner lid cutout
                roundedCube(width-10, height-10, lidDepth+lidLipDepth +1, 6);
            }
            
            lidHoles();
           
        }
    }
    
    if (includeBattery) {
        // Make a battery box...
        // open ended battery box to allow the battery to slide all the 
        // way in without the box catching screw mounts.
        // Sized for a 2000mAh LiPol battery.
        difference() {
            union() {
                // 10mm from edge needed for 
                // base to lid screw pillars
                translate([1.8, 11, 1.5]) {
                    cube([width-3.6, 30, 11.5]);
                }
                
                // Stoppers to prevent the battery falling out
                translate([1.8, 62, 0]) {
                    cube([12, 2, lidDepth+lidLipDepth]);
                }
                
                translate([width - 12 - 1.8, 62, 0]) {
                    cube([12, 2, lidDepth+lidLipDepth]);
                }
            }
            union() {
                translate([1.5+1.7+1.8, 8, 1.5]) {
                    cube([53, 34+5, 10]);
                }
            }
        }
    }
}
    


translate([pcbXOffset, pcbYOffset,pcbZOffset]) {
    %pcbUnderMounted();
}

translate([0,0,35]) {
    lid();
}
