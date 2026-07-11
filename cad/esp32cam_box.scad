toll = 0.25;
toll_lid = 0.08;

wall_th = 2.4;
wall_th_buttom = 4.4;
corner_r_outside = 3;
corner_r_inside =  1.2;

esp_w = toll +  27.5 ;
esp_l = toll +  40.5 ;
esp_h = toll +  17.5 ;

cam_d = 10.5;
cam_w = toll + 10;
cam_h = 2.2;
cam_ox = 10;
cam_oy = esp_w/2;

led_w = toll + 3.2;
led_ox = 30;
led_oy = 25.5;

usb_w = 12;
usb_h = 7;
usb_oz = 0;

pcb_support_h = 1.3;
pcb_oy = 6;

button_w = 4.2;
button_ox = 32.5;
button_oz = 2.5;

lid_h = 4;
lid_overlap = 3;

snap_pos_oc = 10;
snap_w = 2.5;
snap_h = 0.6;
snap_d = 0.4;

wifi_dia = 1.5;
wifi_h = 9.5;

nut_size = 7.7; // m4
nut_th = 3.4; // m4

mount_screw_d = 4.5;
monut_od = 17;
mount_width = 8;
mount_th_overlap = 4;
tessa_size = [25,65];



configuration = -1;


$fn = 400;

if (configuration == -1) {
    mount_screw_h = 8;
        union(){main_body();}
        translate([50,0,0]) rotate([180,0,0])lid1();
        translate([60,20,0])rotate([-90,0,0])wall_mount_part1(height = mount_screw_h);
        translate([130,0,0])rotate([0,0,90])wall_mount_part2();
        translate([60,40,0])rotate([-90,0,0])wall_mount_part1(height = 15);
}

if (configuration == 0) {
    mount_screw_h = 8;
    rotate([0,90,0])
    union(){
        main_body();
        color([0.4,0.5,0.7,0.5])render(convexity = 8)translate([0,0,esp_h])
            lid1();
    
        translate([esp_l/2,-wall_th-mount_width,nut_size/2])rotate([0,90,90])wall_mount_part1(height = mount_screw_h);
        translate([esp_l/2,-monut_od/2-wall_th,-mount_screw_h*2+nut_size/2])rotate([0,90,90])wall_mount_part2();
    }
}

else if (configuration == 1){
    mount_screw_h = 8;
    rotate([0,-90,0])
    union(){

        main_body();
        color([0.4,0.5,0.7,0.5])render(convexity = 8)translate([0,0,esp_h])
            lid1();
        // wall mount
        translate([esp_l/2,esp_w/2,-wall_th_buttom-mount_width])rotate([0,0,-90])wall_mount_part1(height = mount_screw_h);
        translate([esp_l/2,esp_w/2-mount_screw_h*2,-monut_od/2-wall_th_buttom])rotate([180,-90,90])wall_mount_part2();

    }

}
else if (configuration == 2){
    mount_screw_h = 15;
    rotate([0,90,0])
    union(){

        main_body();
        color([0.4,0.5,0.7,0.5])render(convexity = 8)translate([0,0,esp_h])
            lid1();
        // wall mount
        translate([esp_l/2,esp_w/2,-wall_th_buttom-mount_width])rotate([0,0,0])wall_mount_part1(height = mount_screw_h);
        translate([esp_l/2+mount_screw_h*2,esp_w/2,-monut_od/2-wall_th_buttom])rotate([0,-90,0])wall_mount_part2();

    }

}

// main body

// lid

module main_body(){
    difference() {
        hull(){
            translate([-wall_th +    0, -wall_th +     0,  -wall_th_buttom ]) translate([ corner_r_outside, corner_r_outside,0])cylinder(r=corner_r_outside,h=esp_h+wall_th_buttom, $fn=30);
            translate([+wall_th + esp_l,-wall_th +     0,  -wall_th_buttom ]) translate([-corner_r_outside, corner_r_outside,0])cylinder(r=corner_r_outside,h=esp_h+wall_th_buttom, $fn=30);
            translate([+wall_th + esp_l, wall_th + esp_w,  -wall_th_buttom ]) translate([-corner_r_outside,-corner_r_outside,0])cylinder(r=corner_r_outside,h=esp_h+wall_th_buttom, $fn=30);
            translate([-wall_th +     0, wall_th + esp_w,  -wall_th_buttom ]) translate([ corner_r_outside,-corner_r_outside,0])cylinder(r=corner_r_outside,h=esp_h+wall_th_buttom, $fn=30);
        }
    // inside esp cutout    
        translate([corner_r_inside,corner_r_inside,corner_r_inside])
        minkowski(3){
            cube([esp_l-2*corner_r_inside, esp_w-2*corner_r_inside, esp_h]);
            sphere(corner_r_inside, $fn=50);
        } 
    // usb cutout
        translate([esp_l,esp_w/2, usb_h/2 + usb_oz])cube([3*wall_th,usb_w,usb_h], center = true);
    
    // vent holes cutout 
        difference() {
            union()for (i=[0:6]) {
                step = 1.6;
                start_h = usb_oz+step;
                translate([0,esp_w/2 ,start_h+i*step])
                cube([100,18,0.8],center=true);
            }
            translate([1,0,0])cube([100,100,usb_oz+usb_h]);       
        }
    
    // button cutouts 
        translate([button_ox,esp_w/2,esp_h/2+1])cube([button_w,esp_w + 2.1, esp_h+1],center=true);
        translate([button_ox,esp_w/2, 1 + button_oz])cube([button_w,esp_w + 2*wall_th, 2],center=true);

    // 2nd option for mounting through the back.
        translate([esp_l/2,esp_w/2,-50])cylinder(d=mount_screw_d, h=100, $fn = 20);
        translate([esp_l/2,esp_w/2,-nut_th])cylinder(d=nut_size, h=100, $fn = 6);

    //wifi antena cutout
        //translate([esp_l-0.5,esp_w-wifi_dia-1,wifi_h])
        //    cube([wall_th+1,wifi_dia,esp_h]);

    // captive nut cutout
    translate([esp_l/2,-mount_width+0.01,nut_size/2])rotate([-90,0,0])
        cylinder(d=nut_size, h=10, $fn = 6);
    
    }// end of difference

    // add the mount point
    translate([esp_l/2,-wall_th+0.1,nut_size/2])rotate([90,0,0])
    difference(){
        cylinder(d=monut_od-4.1,h = mount_width-3, $fn = 30);
        
        // screw hole
        translate([0,0,-1])cylinder(d = mount_screw_d, h = mount_width+2,$fn = 30);

        // captive nut cutout
        translate([0,0,-0.1])rotate([0,0,0])
        cylinder(d=nut_size, h=nut_th, $fn = 6);

        // chamfer 
        translate([-50,-(monut_od-3)/2,0])rotate([50,0,0])cube([100,10,6],center=false);
    }

    // pcb support 
    translate([1,pcb_oy,0])cube([esp_l-2, wall_th, pcb_support_h]);
    translate([1,esp_w - pcb_oy-wall_th,0])cube([esp_l-2, wall_th, pcb_support_h]);

    // snap tabs
    translate([esp_l/2,-wall_th,esp_h-snap_h]){
        translate([-snap_pos_oc-snap_w/2,-snap_d,0])cube([snap_w,snap_d+1,snap_h]);
        translate([+snap_pos_oc-snap_w/2,-snap_d,0])cube([snap_w,snap_d+1,snap_h]);
    }
    translate([esp_l/2,esp_w+wall_th,esp_h-snap_h]){
        translate([-snap_pos_oc-snap_w/2,-1,0])cube([snap_w,snap_d+1,snap_h]);
        translate([+snap_pos_oc-snap_w/2,-1,0])cube([snap_w,snap_d+1,snap_h]);
    }

    //translate([esp_l/2,esp_w+wall_th-0.05,-wall_th_buttom])
    //    wifi_mount();


}

module lid1(){
    difference(){
            hull(){
                translate([-3*wall_th +    0, -3*wall_th +     0,  -lid_overlap]) translate([ 2*corner_r_outside, 2*corner_r_outside,0])cylinder(r=corner_r_outside,h=lid_h+lid_overlap+wall_th-2, $fn=30);
                translate([+3*wall_th + esp_l,-3*wall_th +     0,  -lid_overlap]) translate([-2*corner_r_outside, 2*corner_r_outside,0])cylinder(r=corner_r_outside,h=lid_h+lid_overlap+wall_th-2, $fn=30);
                translate([+3*wall_th + esp_l, 3*wall_th + esp_w,  -lid_overlap]) translate([-2*corner_r_outside,-2*corner_r_outside,0])cylinder(r=corner_r_outside,h=lid_h+lid_overlap+wall_th-2, $fn=30);
                translate([-3*wall_th +     0, 3*wall_th + esp_w,  -lid_overlap]) translate([ 2*corner_r_outside,-2*corner_r_outside,0])cylinder(r=corner_r_outside,h=lid_h+lid_overlap+wall_th-2, $fn=30);
                
                translate([-3*wall_th +    0, -3*wall_th +     0,  -lid_overlap]) translate([ 2*corner_r_outside, 2*corner_r_outside,0])cylinder(r=corner_r_outside-1.6,h=lid_h+lid_overlap+wall_th, $fn=30);
                translate([+3*wall_th + esp_l,-3*wall_th +     0,  -lid_overlap]) translate([-2*corner_r_outside, 2*corner_r_outside,0])cylinder(r=corner_r_outside-1.6,h=lid_h+lid_overlap+wall_th, $fn=30);
                translate([+3*wall_th + esp_l, 3*wall_th + esp_w,  -lid_overlap]) translate([-2*corner_r_outside,-2*corner_r_outside,0])cylinder(r=corner_r_outside-1.6,h=lid_h+lid_overlap+wall_th, $fn=30);
                translate([-3*wall_th +     0, 3*wall_th + esp_w,  -lid_overlap]) translate([ 2*corner_r_outside,-2*corner_r_outside,0])cylinder(r=corner_r_outside-1.6,h=lid_h+lid_overlap+wall_th, $fn=30);
            }
        
    // inside main body cutout    
        hull(){
            translate([-wall_th +    0, - toll_lid -wall_th +     0-toll_lid,-lid_overlap-wall_th]) translate([ corner_r_outside, corner_r_outside,0])cylinder(r=corner_r_outside,h=lid_overlap+wall_th, $fn=30);
            translate([+wall_th + esp_l + toll_lid,-wall_th +     0-toll_lid,-lid_overlap-wall_th]) translate([-corner_r_outside, corner_r_outside,0])cylinder(r=corner_r_outside,h=lid_overlap+wall_th, $fn=30);
            translate([+wall_th + esp_l + toll_lid, wall_th + esp_w+toll_lid,-lid_overlap-wall_th]) translate([-corner_r_outside,-corner_r_outside,0])cylinder(r=corner_r_outside,h=lid_overlap+wall_th, $fn=30);
            translate([-wall_th +     0 - toll_lid, wall_th + esp_w+toll_lid,-lid_overlap-wall_th]) translate([ corner_r_outside,-corner_r_outside,0])cylinder(r=corner_r_outside,h=lid_overlap+wall_th, $fn=30);
        }
    // inside main body cutout    
        hull(){
            translate([         toll_lid,       toll_lid,-lid_overlap-wall_th]) translate([ corner_r_outside, corner_r_outside,0])cylinder(r=corner_r_outside,h=lid_overlap+lid_h+wall_th, $fn=30);
            translate([ esp_l + toll_lid,       toll_lid,-lid_overlap-wall_th]) translate([-corner_r_outside, corner_r_outside,0])cylinder(r=corner_r_outside,h=lid_overlap+lid_h+wall_th, $fn=30);
            translate([ esp_l + toll_lid, esp_w+toll_lid,-lid_overlap-wall_th]) translate([-corner_r_outside,-corner_r_outside,0])cylinder(r=corner_r_outside,h=lid_overlap+lid_h+wall_th, $fn=30);
            translate([         toll_lid, esp_w+toll_lid,-lid_overlap-wall_th]) translate([ corner_r_outside,-corner_r_outside,0])cylinder(r=corner_r_outside,h=lid_overlap+lid_h+wall_th, $fn=30);
        }
       
    // cam cutout
        translate([cam_ox,cam_oy,-1])cylinder(d=cam_d,h=lid_h+wall_th+2, $fn=50);
    
    // led cutout
        hull(){
            translate([led_ox,led_oy,lid_h])cube([led_w,led_w,1],center = true);
            translate([led_ox,led_oy,lid_h+wall_th+1])cube([led_w*2,led_w*2,1],center = true);

        }
    // snap tabs
    snap_d2 = snap_d+0.3;
    snap_w2 = snap_w+0.4;
    snap_h2 = snap_h+0.3;

        translate([esp_l/2,-wall_th,-snap_h2]){
            translate([-snap_pos_oc-snap_w2/2,-snap_d2,0])cube([snap_w2,snap_d2+1,snap_h2]);
            translate([+snap_pos_oc-snap_w2/2,-snap_d2,0])cube([snap_w2,snap_d2+1,snap_h2]);
        }
        translate([esp_l/2,esp_w+wall_th,-snap_h2]){
            translate([-snap_pos_oc-snap_w2/2,-1,0])cube([snap_w2,snap_d2+1,snap_h2]);
            translate([+snap_pos_oc-snap_w2/2,-1,0])cube([snap_w2,snap_d2+1,snap_h2]);
        }

    }

    // cam support
    difference() {
        translate([cam_ox-cam_w/2-wall_th,cam_oy-cam_w/2-wall_th,cam_h])cube([cam_w+2*wall_th, cam_w+2*wall_th, lid_h+wall_th-cam_h-0.1]);
        translate([cam_ox,cam_oy,1])cylinder(d=cam_d,h=lid_h+wall_th+2,$fn=50);
        
    }



}


*mount();
module mount(nut = true, height = 10){
    $fn = 30;
    difference() {
        hull() {
            translate([0,0,height])rotate([-90,0,0])cylinder(d = monut_od, h = mount_width);
            translate([-monut_od/2,0,0])cube([monut_od,mount_width,0.1]);
        }
        
        translate([-monut_od,-mount_width+1,-100])cube([monut_od*2,mount_width*2,100]);
        
        // screw hole
        translate([0,-0.1,height])rotate([-90,0,0])cylinder(d = mount_screw_d, h = mount_width+1);

        if (nut) {
            // captive nut cutout
            translate([0,mount_width-nut_th,height])rotate([-90,0,0])cylinder(d=nut_size, h=10, $fn = 6);            
        }


    }

}


module wifi_mount() {
    hole_d = 6.8;
    w = 13;
    th = 5;
    fillet_r = 2;
    o = 3;
    l = w + o;
    $fn = 30;
    rotate([0,-90,0])difference(){
        translate([0,0,0])cube([w,l,th]);
        translate([w/2,w/2+o,-0.1])cylinder(d=hole_d, h=th+1);
        translate([w,  l, th/2])rotate(180)fillet(fillet_r,th+1);
        translate([0, l, th/2])rotate(-90)fillet(fillet_r,th+1);
    }
    translate([-th, 0, w/2])rotate(90)fillet(fillet_r,w);
    translate([0, 0, w/2])rotate(0)fillet(fillet_r,w);
    difference() {
        translate([-th/2, 0, w])rotate([0,-90,0])fillet(2,th+4);
        translate([-th, 0, w/2])rotate(90)translate([fillet_r,fillet_r,0])cylinder(r=fillet_r,h=w+4,center=true);
        translate([0, 0, w/2])           translate([fillet_r,fillet_r,0])cylinder(r=fillet_r,h=w+4,center=true);
    }
}

*wall_mount_part1();
module wall_mount_part1(height = 10) {
    $fn  = 50;
    mount_screw_h = height;
    union(){
        // Create the mount attachment point 
        difference() {
            union() {
                translate([mount_screw_h,0,0])rotate([90,0,-90])mount(nut = false, height = height);
                
                // add thickness to back
                translate([mount_screw_h-mount_th_overlap/2,-monut_od/2,-monut_od+mount_width])cube([mount_th_overlap, monut_od, monut_od]);

                // small fillet 
                translate([mount_screw_h+mount_th_overlap/2,monut_od/2-mount_width,-monut_od/2+mount_width])rotate(-90)fillet(2,monut_od);
                translate([mount_screw_h-mount_th_overlap/2,0,0])rotate([-90,0,180])fillet(2,monut_od);

                // 2nd mount point 
                translate([mount_screw_h,monut_od/2-mount_width,-monut_od/2+mount_width])rotate([0,90,0])mount(nut = false, height = height);
            }
            
            // create the hole for the mount point on the cam body 
            translate([0,0,3])
            cylinder(d = monut_od-4, h = mount_width);

            // screw clerance in fillet 
            translate([0,0,0.5])rotate([180,0,0])cylinder(d=mount_screw_d*2.5, h = 10);
            
            // cylinder to cut away fillets
            translate([mount_screw_h*2,monut_od/2-mount_width,mount_width-monut_od/2])rotate([90,0,0])cylinder(d = monut_od+1, h = 50);
            translate([mount_screw_h*2,monut_od/2-mount_width,mount_width-monut_od/2])rotate([180,90,0])cube([monut_od,monut_od,(monut_od+1)/2]);
            

            // fillet on corner
            *translate([monut_od/2,-monut_od/2,mount_width-monut_od-0.1])rotate([90,0,90])fillet(monut_od/2,mount_screw_h*2);
            translate([mount_screw_h/2,-monut_od/2,mount_width-monut_od])rotate([0,-90,180])resize([(monut_od-mount_width)*2,monut_od*2-mount_width*2])cylinder(d =1, h = mount_screw_h);

        }



    } // end union
}

*wall_mount_part2();
module wall_mount_part2(){
    $fn  = 30;
    tessa_shape_th = 3;
    translate([0,0,-1])
    union(){
        
        translate([0,monut_od/2-mount_width,-monut_od/2])rotate([0,0,180])mount(nut = true, height = monut_od/2+1);

        // Add a protrusion to the mount 
        translate([monut_od/2,monut_od/2-mount_width,-tessa_shape_th-monut_od/2])rotate([0,0,180])cube([monut_od, mount_width, tessa_shape_th+1]);
       

        // tessa glue strip backblate 
 
        hull(){
            translate([0+2,-tessa_size[1]/2+2,-monut_od/2-0.1])cube([tessa_size[0]-4,tessa_size[1]-4,0.1]);
            translate([0,-tessa_size[1]/2,-monut_od/2-tessa_shape_th])cube([tessa_size[0],tessa_size[1],0.1]);
            translate([0,monut_od/2-mount_width-monut_od/2-mount_width,-monut_od/2-tessa_shape_th])cube([4,monut_od+mount_width,tessa_shape_th]);
        }  
        translate([0.1,-mount_width+monut_od/2-mount_width,-monut_od/2-tessa_shape_th/2])rotate(180)fillet(monut_od/2,tessa_shape_th);
        translate([0.1,monut_od/2-mount_width,-monut_od/2-tessa_shape_th/2])rotate(90)fillet(monut_od/2,tessa_shape_th);
        
    }      

    
}







module fillet(r,thickness) {
    translate([r / 2, r / 2, 0])

        difference() {
            cube([r + 0.01, r + 0.01,thickness], center = true);

            translate([r/2, r/2, 0])
                cylinder(r =r,h=thickness+0.01, center = true);
        }
}

