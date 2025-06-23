#include "math.hh"
#include "vchambers.hh"

void vc::VChambers::create_item(int id, int x, int y) {
	Item item;
	item.id = id;
	item.y = math::clamp(y, 0, (int)map.size() - 2);
	item.x = math::clamp(x, 0, map.at(0).size() - 2);

	item_list.push_back(item);
}
