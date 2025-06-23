#include "vchambers.hh"

void vc::VChambers::create_entitytile(int x, int y, int id) {
	EntityTile et;
	et.x = x;
	et.y = y;
	et.id = id;
	et.health = 100;
	entitytile_list.push_back(et);
}
