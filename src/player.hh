#pragma once
#include <vector>

namespace vc {
struct Player {
	int x;
	int y;
	int health;
	double dhealth;
	int ammo;
	int dash_count;
	bool dashing;
	std::vector<int> inventory;
};
} // namespace vc
