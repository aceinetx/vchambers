#pragma once

namespace vc {
struct Entity {
	int x;
	int y;
	int health;
	int entity_id;
	bool walking_path;
	double dx;
	double dy;
};
} // namespace vc
