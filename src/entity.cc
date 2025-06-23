#include "math.hh"
#include "vchambers.hh"

void vc::VChambers::create_entity(int id, int x, int y) {
	Entity e;
	e.y = math::clamp(y, 0, (int)map.size() - 2);
	e.x = math::clamp(x, 0, (int)map.at(0).size() - 2);
	e.dx = e.x;
	e.dy = e.y;
	e.health = 100;
	e.walking_path = false;
	e.entity_id = 0;
	entity_list.push_back(e);
}
