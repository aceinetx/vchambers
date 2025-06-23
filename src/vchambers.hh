#pragma once
#include "door.hh"
#include "entity.hh"
#include "entitytile.hh"
#include "item.hh"
#include "json.hpp"
#include "ncurses.h"
#include "player.hh"
#include "vcvec.hh"
#include <fstream>

namespace vc {
class VChambers {
	Player player;			 // initalize player
	nlohmann::json data; // initalize data
	int scene = 0;
	int menu_selection = 0;
	float angle;
	float fov = 3.1415926;
	float depth = 9.0f;
	int entity_damage_range = 2;
	int selected_item = 0;
	int render_range = 5;
	float aim_sensivity = 0.2;
	bool debug = false;
	int level = 0;
	bool aiming = false;
	bool silent_aim = false;
	int last_move = 0;

	std::ifstream f;

	// strings
	const std::string menu_title = "VChambers";
	const std::string menu_play = "Play";
	const std::string menu_exit = "Exit";
	const std::string secret_revealed = "A secret is revealed!";

	// vectors
	std::vector<std::string> map;
	std::vector<char> collision_tiles = {'#'};
	std::vector<Entity*> ray_entities;
	std::vector<VCVec2> rays;
	std::vector<Entity> entity_list;
	std::vector<Item> item_list;
	std::vector<Door> door_list;
	std::vector<EntityTile> entitytile_list;
	std::vector<EntityTile*> ray_entitytiles;
	MEVENT ev;

	enum COLORS {
		BUTTON = 1,
		BUTTON_CLICK = 2,
		AIM_RAY = 3,
		ITEM = 4,
		WALL = 5,
		GROUND = 6,
		SECRET_REVEALED = 7,
	};

	enum DOOR {
		DOOR_CLOSED = '$',
		DOOR_OPEN = '[',
	};

public:
	void initalize();
	void init_level(bool first_init = false);
	void save_json();

	void create_door(int x, int y);
	void create_entitytile(int x, int y, int id);
	void create_entity(int id, int x, int y);
	void create_item(int id, int x, int y);

	void end();

	bool collides(char tile, int y, int x);

	bool in_range(int value, int _min, int _max);

	int clamp(int value, int _min, int _max);

	int random(int _min, int _max);

	void game_thread();

	std::string health_bar(int health);

	void render();

	~VChambers();
};
} // namespace vc
