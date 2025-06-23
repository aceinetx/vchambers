#include "vchambers.hh"

void vc::VChambers::create_door(int x, int y) {
	Door d;
	d.x = x;
	d.y = y;
	d.opened = false;
	door_list.push_back(d);
}
