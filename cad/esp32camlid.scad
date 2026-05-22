$fn = 64;


difference(){
	difference() {
		union() {
			hull() {
				union() {
					translate(v = [21.2500000000, 15.0000000000, 0]) {
						cylinder(h = 4.2000000000, r = 2);
					}
					translate(v = [-21.2500000000, 15.0000000000, 0]) {
						cylinder(h = 4.2000000000, r = 2);
					}
					translate(v = [21.2500000000, -15.0000000000, 0]) {
						cylinder(h = 4.2000000000, r = 2);
					}
					translate(v = [-21.2500000000, -15.0000000000, 0]) {
						cylinder(h = 4.2000000000, r = 2);
					}
				}
			}
			translate(v = [0, 0, 4.1990000000]) {
				difference() {
					hull() {
						union() {
							translate(v = [19.8750000000, 13.6250000000, 0]) {
								cylinder(h = 5, r = 2);
							}
							translate(v = [-19.8750000000, 13.6250000000, 0]) {
								cylinder(h = 5, r = 2);
							}
							translate(v = [19.8750000000, -13.6250000000, 0]) {
								cylinder(h = 5, r = 2);
							}
							translate(v = [-19.8750000000, -13.6250000000, 0]) {
								cylinder(h = 5, r = 2);
							}
						}
					}
					translate(v = [0, 0, -0.5000000000]) {
						translate(v = [-20.2500000000, -14.0000000000, 0]) {
							cube(size = [40.5000000000, 28, 6]);
						}
					}
				}
			}
			translate(v = [-17.7500000000, 0, 4.1990000000]) {
				translate(v = [-2.5000000000, -14.0000000000, 0]) {
					cube(size = [5, 28, 2]);
				}
			}
		}
		translate(v = [10, 0, -0.0020000000]) {
			union() {
				cylinder(d = 8.5000000000, h = 10);
				cylinder(d1 = 12, d2 = 8.5000000000, h = 0);
				translate(v = [0, 0, 1.7000000000]) {
					translate(v = [-4.5000000000, -4.5000000000, 0]) {
						cube(size = [9, 9, 3.5000000000]);
					}
				}
			}
		}
		translate(v = [0, 0, 5]) {
			hull() {
				union() {
					translate(v = [5.5000000000, 17.0000000000, 2]) {
						sphere(r = 2);
					}
					translate(v = [-5.5000000000, 17.0000000000, 2]) {
						sphere(r = 2);
					}
					translate(v = [5.5000000000, -17.0000000000, 2]) {
						sphere(r = 2);
					}
					translate(v = [-5.5000000000, -17.0000000000, 2]) {
						sphere(r = 2);
					}
					translate(v = [5.5000000000, 17.0000000000, 8]) {
						sphere(r = 2);
					}
					translate(v = [-5.5000000000, 17.0000000000, 8]) {
						sphere(r = 2);
					}
					translate(v = [5.5000000000, -17.0000000000, 8]) {
						sphere(r = 2);
					}
					translate(v = [-5.5000000000, -17.0000000000, 8]) {
						sphere(r = 2);
					}
				}
			}
		}
		union() {
			translate(v = [21.7500000000, 0, 7.2000000000]) {
				hull() {
					union() {
						translate(v = [0, 11.2000000000, 0]) {
							sphere(r = 0.8000000000);
						}
						translate(v = [0, -11.2000000000, 0]) {
							sphere(r = 0.8000000000);
						}
					}
				}
			}
			translate(v = [-21.7500000000, 0, 7.2000000000]) {
				hull() {
					union() {
						translate(v = [0, 11.2000000000, 0]) {
							sphere(r = 0.8000000000);
						}
						translate(v = [0, -11.2000000000, 0]) {
							sphere(r = 0.8000000000);
						}
					}
				}
			}
		}
	}
	/* Holes Below*/
	union(){
		translate(v = [-9.5000000000, 11.2500000000, -5]) {
			hull() {
				union() {
					translate(v = [1.2500000000, 1.2500000000, 0]) {
						cylinder(h = 20, r = 0.5000000000);
					}
					translate(v = [-1.2500000000, 1.2500000000, 0]) {
						cylinder(h = 20, r = 0.5000000000);
					}
					translate(v = [1.2500000000, -1.2500000000, 0]) {
						cylinder(h = 20, r = 0.5000000000);
					}
					translate(v = [-1.2500000000, -1.2500000000, 0]) {
						cylinder(h = 20, r = 0.5000000000);
					}
				}
			}
		}
	} /* End Holes */ 
}