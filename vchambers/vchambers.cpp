#include <iostream>
#include <Windows.h>
#include <curses.h>
#include <vector>
#include <thread>
#include <string>
#include <fstream>
#include "json.hpp"
#include <math.h>

using std::cout, std::endl, std::vector, std::string, std::wstring, std::thread, std::to_string, std::ostream, std::ifstream, nlohmann::json;

std::ifstream f("VChambers.json");

HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

namespace ItemTexts {
    char get_item_char(int id) {
        switch (id) {
        case 1: return '/';
        case 2: return '+';
        case 3: return '|';
        case 4: return '*';
        }
        return to_string(id).at(0);
    }

    string get_item_name(int id) {
        switch (id) {
        case 1: return "Piston";
        case 2: return "Medkit";
        case 3: return "Ammo";
        case 4: return "Shotgun";
        }
        return "ItemId." + to_string(id);
    }
};


struct Player {
    int x;
    int y;
    int health;
    double dhealth;
    int ammo;
    int dash_count;
    bool dashing;
    vector<int> inventory;
};

struct Entity {
    int x;
    int y;
    int health;
    int entity_id;
    bool walking_path;
    double dx; // NOT DELTA!!
    double dy;
};

struct EntityTile {
    int x;
    int y;
    int id;
    int health;
};

struct Item {
    int id;
    int x;
    int y;
};

struct VCVec2 {
    int x;
    int y;
};

struct Door {
    int x;
    int y;
    bool opened;
};


class VChambers {

    Player player = Player(); // initalize player
    json data; // initalize data
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
    int level;
    bool aiming = false;
    bool silent_aim = false;
    int last_move = 0;
    int _; // temporary variable

    // strings
    string menu_title = "VChambers";
    string menu_play = "Play";
    string menu_exit = "Exit";
    string secret_revealed = "A secret is revealed!";

    // vectors
    vector<string> map;
    vector<char> collision_tiles = { '#' };
    vector<Entity*> ray_entities;
    vector<VCVec2> rays;
    vector<Entity> entity_list;
    vector<Item> item_list;
    vector<Door> door_list;
    vector<EntityTile> entitytile_list;
    vector<EntityTile*> ray_entitytiles;
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

    void initalize() {

        srand((unsigned)time(NULL)); // random seed

        ev.x = 0;
        ev.y = 0;
        player.x = 1;
        player.y = 1;
        player.health = 100;
        player.dhealth = 100.0;


        try {
            data = json::parse(f);
            f.close();
        }
        catch (...) {
            std::ofstream n("VChambers.json");
            n << "{}";
            n.close();

            std::ifstream file("VChambers.json");
            data = json::parse(file);
            file.close();
        }

        if (!data["aim_sensivity"].is_number_float()) {
            data["aim_sensivity"] = 0.2;
        }
        else { aim_sensivity = data["aim_sensivity"].template get<float>(); }

        if (!data["px"].is_number()) {
            data["px"] = player.x;
        }
        else { player.x = data["px"].template get<int>(); }

        if (!data["py"].is_number_integer()) {
            data["py"] = player.y;
        }
        else { player.y = data["py"].template get<int>(); }

        if (!data["ammo"].is_number_integer()) {
            data["ammo"] = 0;
        }
        else { player.ammo = data["ammo"].template get<int>(); }

        if (!data["level"].is_number_integer()) {
            data["level"] = 1;
        }
        else { level = data["level"].template get<int>(); }

        if (!data["inventory"].is_array()) {
            data["inventory"] = json::array();
        }
        else {
            for (json& i : data["inventory"].template get<json>()) {
                player.inventory.push_back(i.template get<int>());
            }
        }

        if (!data["item_list"].is_array()) {
            data["item_list"] = json::array();
        }
        else {
            for (json& i : data["item_list"].template get<json>()) {
                Item itm;
                itm.x = i.at(0).template get<int>();
                itm.y = i.at(1).template get<int>();
                itm.id = i.at(2).template get<int>();
                item_list.push_back(itm);
            }
        }

        //data["entitytile_list"] = 1;

        if (!data["entitytile_list"].is_array()) {
            data["entitytile_list"] = json::array();
        }
        else {
            for (json& i : data["entitytile_list"].template get<json>()) {
                EntityTile et;
                et.x = i.at(0).template get<int>();
                et.y = i.at(1).template get<int>();
                et.id = i.at(2).template get<int>();
                et.health = i.at(3).template get<int>();
                entitytile_list.push_back(et);
            }
        }

        if (!data["entity_list"].is_array()) {
            data["entity_list"] = json::array();
        }
        else {
            for (json& i : data["entity_list"].template get<json>()) {
                Entity e;
                e.dx = i[0].template get<double>();
                e.dy = i[1].template get<double>();
                e.entity_id = i[2].template get<int>();
                e.health = i[3].template get<int>();
                entity_list.push_back(e);
            }
        }

        if (!data["door_list"].is_array()) {
            data["door_list"] = json::array();
        }
        else {
            for (json& i : data["door_list"].template get<json>()) {
                Door d;
                d.x = i[0].template get<int>();
                d.y = i[1].template get<int>();
                d.opened = i[2].template get<bool>();
                door_list.push_back(d);
            }
        }

        player.dashing = false;
        player.dash_count = 0;

        save_json();

        thread gamethread(&VChambers::game_thread, this);
        gamethread.detach();



        SetConsoleTextAttribute(hStdOut, 0xc);
        cout << "<<<=========----------------- VChambers -----------------=========>>>\n";
        cout << "<<<=========                                            -=========>>>\n";
        cout << "<<<=========                  Controls:                 -=========>>>\n";
        cout << "<<<=========              W, A, S, D  - Move            -=========>>>\n";
        cout << "<<<=========              LEFT, RIGHT - Aim             -=========>>>\n";
        cout << "<<<=========              UP, DOWN    - Choose item     -=========>>>\n";
        SetConsoleTextAttribute(hStdOut, 0xb);
        cout << "<<<=========         Modify settings in VChambers.json  -=========>>>\n";
        cout << "<<<=========                                            -=========>>>\n";
        cout << "<<<=========                                            -=========>>>\n";
        cout << "<<<=========              Press ENTER to play           -=========>>>\n";
        cout << "<<<=========                                            -=========>>>\n";
        cout << "<<<=========               Made by Aceinet              -=========>>>\n";
        cout << "<<<=========-----------------...........-----------------=========>>>\n";
        SetConsoleTextAttribute(hStdOut, 7);
        int k = getchar();
        if (k == 'q') {
            debug = true;
        }

        initscr();


        noecho();
        nodelay(stdscr, true);
        keypad(stdscr, true);

        if (has_colors() == FALSE)
        {
            end();
            cout << "[ERR] Your terminal does not support colors" << endl;
            exit(1);
        }

        start_color();
        init_pair(1, COLOR_BLUE, COLOR_BLACK);
        init_pair(2, COLOR_BLACK, COLOR_BLUE);
        init_pair(3, COLOR_CYAN, COLOR_BLACK);
        init_pair(4, 240, COLOR_BLACK);
        init_pair(5, COLOR_RED, COLOR_BLACK);
        init_pair(6, 155, COLOR_BLACK);
        init_pair(7, 227, COLOR_BLACK);

        init_level(true);

        curs_set(0);
    }

    void init_level(bool first_init = false) {

        if (!first_init) {
            entity_list.clear();
            door_list.clear();
            item_list.clear();
            entitytile_list.clear();
            map.clear();
        }

        switch (level) {
        case 0:
            map.push_back("#####################################################################");
            map.push_back("#......................................#N......................#ssss#");
            map.push_back("#......................................#.......................#ssss#");
            map.push_back("#......................................#.......................#ssss#");
            map.push_back("#......................................#.......................#ssss#");
            map.push_back("#......................................#.......................#ssss#");
            map.push_back("#......................................##############..############.#");
            map.push_back("#......................................,............#...............#");
            map.push_back("#......................................,............#...............#");
            map.push_back("#......................................###########..#######..########");
            map.push_back("#......................................#.........#..................#");
            map.push_back("#####################################################################");

            if (!first_init) {
                create_entity(0, random(10, 20), random(8, 9));
                create_entity(0, random(52, 70), random(1, 5));

                create_item(1, 1, 10);
                create_item(2, 20, 10);
                create_item(3, 21, 10);
                create_item(3, 55, 8);

                create_door(67, 6);

                player.dhealth = 100;
                player.x = 1;
                player.y = 1;
            }

            break;
        case 1:
            map.push_back("#####################################################################");
            map.push_back("#.......#..........#............................#...................#");
            map.push_back("#.......#.......................#...............##############..#####");
            map.push_back("#.......#..........######.#######...................................#");
            map.push_back("#######.#..........#............#...............#####################");
            map.push_back("#.....#..########.##............#...............#...................#");
            map.push_back("#.....#............#............###############.#...................#");
            map.push_back("#.....#............#............#...............#...................#");
            map.push_back("#.....#.####.......#............#...............###.................#");
            map.push_back("#..........#.......#............#.................#.................#");
            map.push_back("#..........#.......#............#...................................#");
            map.push_back("##################################################################.##");
            map.push_back("#.....................................................###sssssssssss#");
            map.push_back("#.....................................................###sssssssssss#");
            map.push_back("#####################################################################");

            if (!first_init) {
                create_door(7, 4);
                create_door(7, 8);
                create_door(25, 3);
                create_door(47, 6);
                create_door(66, 11);

                create_entity(0, 25, 7);
                create_entity(0, 25, 8);
                create_entity(0, 7, 7);

                create_item(3, 55, 1);
                create_item(3, 56, 1);
                create_item(2, 57, 1);
                create_item(4, 63, 13);

                create_entitytile(12, 2, 1);
                player.dhealth = 100;
                player.x = 1;
                player.y = 1;
            }

            break;
        default:
            end();
            cout << "[VChambers] Invaild level\n";
            exit(1);
        }

    }

    void save_json() {

        data["px"] = player.x;
        data["py"] = player.y;
        data["level"] = level;
        data["ammo"] = player.ammo;

        data["inventory"] = json::array();
        for (int& item : player.inventory) {
            data["inventory"].push_back(item);
        }

        data["item_list"] = json::array();
        for (Item& i : item_list) {
            data["item_list"].push_back(json::array({ i.x, i.y, i.id }));
        }

        data["entity_list"] = json::array();
        for (Entity& i : entity_list) {
            data["entity_list"].push_back(json::array({ i.dx, i.dy, i.entity_id, i.health }));
        }

        data["entitytile_list"] = json::array();
        for (EntityTile& i : entitytile_list) {
            data["entitytile_list"].push_back(json::array({ i.x, i.y, i.id, i.health }));
        }

        data["door_list"] = json::array();
        for (Door& i : door_list) {
            data["door_list"].push_back(json::array({ i.x, i.y, i.opened }));
        }


        std::ofstream n("VChambers.json");
        n << data.dump();
        n.close();
    }

    void create_door(int x, int y) {
        Door d;
        d.x = x;
        d.y = y;
        d.opened = false;
        door_list.push_back(d);
    }

    void create_entitytile(int x, int y, int id) {
        EntityTile et;
        et.x = x;
        et.y = y;
        et.id = id;
        et.health = 100;
        entitytile_list.push_back(et);
    }

    void create_entity(int id, int x, int y) {
        Entity e;
        e.y = clamp(y, 0, (int)map.size() - 2);
        e.x = clamp(x, 0, (int)map.at(0).size() - 2);
        e.dx = e.x;
        e.dy = e.y;
        e.health = 100;
        e.walking_path = false;
        e.entity_id = 0;
        entity_list.push_back(e);
    }

    void create_item(int id, int x, int y) {
        Item item; item.id = id; item.y = clamp(y, 0, (int)map.size() - 2); item.x = clamp(x, 0, map.at(0).size() - 2);

        item_list.push_back(item);
    }

    void end() {
        endwin();
    }

    bool collides(char tile, int y, int x) {
        for (char& c : collision_tiles) {
            if (c == tile) {
                return true;
            }
            else {
                for (Door& door : door_list) {
                    if (door.x == x && (door.y == y)) {
                        if (door.opened == false) return true;
                    }
                }

                for (EntityTile& et : entitytile_list) {
                    if (et.x == x && (et.y == y)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool in_range(int value, int _min, int _max) {
        if (value >= _min && (value <= _max)) return true;
        return false;
    }

    int clamp(int value, int _min, int _max) {
        if (value < _min) return _min;
        if (value > _max) return _max;
        return value;
    }

    int random(int _min, int _max) {
        int result = _min + (rand() % _max + 1);

        srand((unsigned)time(NULL));

        return result;
    }

    void game_thread() {
        while (true) {
            if (scene == 1) {
                if (player.health <= 0) {
                    //scene = 2;
                    //continue;
                }
                int i = 0;

                if (map.at(player.y).at(player.x) == 'N') {
                    continue;
                }
                for (Entity& entity : entity_list) {
                    if (map.at(player.y).at(player.x) == 'N') {
                        continue;
                    }

                    VCVec2 entity_pos;
                    VCVec2 player_pos;

                    entity.x = clamp(entity.x, 1, map.at(0).size() - 2);
                    entity.y = clamp(entity.y, 1, map.size() - 2);

                    entity_pos.x = entity.x;
                    entity_pos.y = entity.y;

                    player_pos.x = player.x;
                    player_pos.y = player.x;


                    if (entity.entity_id == 0) {
                        if (in_range(player.x, entity.x - entity_damage_range, entity.x + entity_damage_range) && (in_range(player.y, entity.y - entity_damage_range, entity.y + entity_damage_range))) {
                            player.dhealth -= 0.3;
                        }
                    }

                    if (entity.health <= 0) {
                        entity_list.erase(entity_list.begin() + i);
                    }

                    float ea = 7;
                    bool player_found = false;
                    while (ea < 6) {
                        float edist = 0.0;
                        float e_eye_x = sin(ea);
                        float e_eye_y = cos(ea);
                        bool break_l = false;
                        while (edist < 1) {
                            int eray_x = entity.x + e_eye_x * edist;
                            int eray_y = entity.y + e_eye_y * edist;
                            eray_x = clamp(eray_x, 0, map.at(0).size() - 2);
                            eray_y = clamp(eray_y, 0, map.size() - 2);

                            if (collides(map.at(eray_y).at(eray_x), eray_y, eray_x)) {
                                break_l = true;
                                break;
                            }

                            if (eray_x == player.x && (eray_y == player.y)) {
                                player_found = true;
                                break;
                            }
                            edist += 0.1;
                        }
                        if (player_found) break;
                        if (break_l) break;
                    }

                    int move = random(1, 8) / 2;
                    if (player_found) move = -1;
                    int old_x = entity.dx;
                    int old_y = entity.dy;
                    switch (move) {
                    case 1:
                        entity.dx += .2;
                        break;
                    case 2:
                        entity.dx -= .2;
                        if (random(0, 1)) {
                            entity.dy += .2;
                        }
                        break;
                    case 3:
                        entity.dy += .2;
                        break;
                    case 4:
                        entity.dy -= .2;
                        break;
                    }


                    if (collides(map.at((int)entity.dy).at((int)entity.dx), entity.dy, entity.dx)) {
                        entity.dx = (double)old_x;
                        entity.dy = (double)old_y;
                    }
                    entity.x = (int)entity.dx;
                    entity.y = (int)entity.dy;


                    entity.x = clamp(entity.x, 1, map.at(0).size() - 2);
                    entity.y = clamp(entity.y, 1, map.size() - 2);

                    i++;
                }

                int j = 0;
                for (Item& item : item_list) {
                    if (map.at(player.y).at(player.x) == 'N') {
                        continue;
                    }
                    item.x = clamp(item.x, 1, map.at(0).size() - 1);
                    item.y = clamp(item.y, 1, map.size() - 1);
                    if (item.x == player.x && (item.y == player.y)) {
                        player.inventory.push_back(item.id);
                        item_list.erase(item_list.begin() + j);
                        continue;
                    }
                    j++;
                }

                for (Door& d : door_list) {
                    if (map.at(player.y).at(player.x) == 'N') {
                        continue;
                    }
                    d.x = clamp(d.x, 1, map.at(0).size() - 1);
                    d.y = clamp(d.y, 1, map.size() - 1);
                }

                i = 0;
                for (EntityTile& et : entitytile_list) {
                    if (et.health <= 0) entitytile_list.erase(entitytile_list.begin() + i);
                    i++;
                }

                if (player.dashing) {
                    if (player.dash_count <= 0) player.dashing = false;
                    else {
                        if (last_move == 0) {
                            player.x -= 1;
                            if (collides(map.at(player.y).at(player.x), player.y, player.x)) {
                                player.dashing = false;
                                player.dash_count = 0;
                                player.x += 1;
                            }
                            else player.dash_count -= 1;
                        }
                        else if (last_move == 1) {
                            player.x += 1;
                            if (collides(map.at(player.y).at(player.x), player.y, player.x)) {
                                player.dashing = false;
                                player.dash_count = 0;
                                player.x -= 1;
                            }
                            else player.dash_count -= 1;
                        }
                    }
                }

                player.health = (int)player.dhealth;
            }
            Sleep(50);
        }
        cout << "[WARNING] Thread stopped";
    }

    string health_bar(int health) {
        string buf;
        for (int i = 0; i < (int)health / 10; i++) {
            buf.push_back('=');
        }
        for (int i = 0; i < (10 - (int)health / 10); i++) {
            buf.push_back('.');
        }
        return buf;
    }

    void render() {

        if (menu_selection <= -1) {
            menu_selection = 0;
        }
        if (menu_selection >= 2) {
            menu_selection = 1;
        }


        clear();

        int max_y = getmaxy(stdscr);
        int max_x = getmaxx(stdscr);

        if (scene == 1) {
            if (map.at(player.y).at(player.x) == 'N') {
                level++;
                init_level();
            }
        }

        if (scene == 0) {
            mvprintw(0, (max_x / 2 - menu_title.length() / 2), menu_title.c_str());

            mvprintw(1, (max_x / 2 - menu_play.length() / 2), menu_play.c_str());

            mvprintw(2, (max_x / 2 - menu_exit.length() / 2), menu_exit.c_str());

            if (menu_selection == 0) {
                attron(COLOR_PAIR(COLORS::BUTTON));
                mvprintw(1, (max_x / 2 - menu_play.length() / 2), menu_play.c_str());
                mvprintw(1, (max_x / 2 - menu_exit.length() / 2), "");
                attroff(COLOR_PAIR(COLORS::BUTTON));
            }
            else if (menu_selection == 1) {
                attron(COLOR_PAIR(COLORS::BUTTON));
                mvprintw(2, (max_x / 2 - menu_exit.length() / 2), menu_exit.c_str());
                mvprintw(2, (max_x / 2 - menu_exit.length() / 2), "");
                attroff(COLOR_PAIR(COLORS::BUTTON));
            }
        }
        else if (scene == 1) {
            float render_angle = 0.0f;
            while (render_angle < 16) {
                float render_distance = 0.0;
                float render_eye_x = sin(render_angle);
                float render_eye_y = cos(render_angle);

                while (render_distance < render_range) {
                    VCVec2 render_ray;
                    render_ray.x = (int)player.x + render_eye_x * render_distance;
                    render_ray.y = (int)player.y + render_eye_y * render_distance;
                    render_ray.x = clamp(render_ray.x, 0, map.at(0).length() - 1);
                    render_ray.y = clamp(render_ray.y, 0, map.size() - 1);


                    char map_char = map.at(render_ray.y).at(render_ray.x);
                    short color = 0;
                    switch (map_char) {
                    case '#':
                        color = COLORS::WALL;
                        break;

                    case '.':
                        color = COLORS::GROUND;
                        break;

                    case 's':
                        color = COLORS::GROUND;
                        map_char = '.';
                        break;
                    }
                    attron(COLOR_PAIR(color));
                    mvprintw(render_ray.y, render_ray.x, "%c", map_char);
                    attroff(COLOR_PAIR(color));
                    map_char = map.at(render_ray.y).at(render_ray.x);
                    if (map_char == 's') {
                        if (render_ray.x == player.x && (render_ray.y == player.y)) { // secret revealed
                            attron(COLOR_PAIR(COLORS::SECRET_REVEALED));
                            mvprintw(max_y - 1, max_x - 1 - secret_revealed.length(), secret_revealed.c_str());
                            attroff(COLOR_PAIR(COLORS::SECRET_REVEALED));
                        }
                    }

                    for (Door& door : door_list) {
                        if (door.x == render_ray.x && (door.y == render_ray.y)) {
                            if (door.opened == false) mvprintw(door.y, door.x, "%c", DOOR::DOOR_CLOSED);
                            else if (door.opened == true) mvprintw(door.y, door.x, "%c", DOOR::DOOR_OPEN);
                            else {
                                mvprintw(door.y, door.x, "?");
                            }
                        }
                    }

                    for (EntityTile& et : entitytile_list) {
                        if (et.id == 1) {
                            if (et.x == render_ray.x && (et.y == render_ray.y)) {
                                attron(COLOR_PAIR(COLORS::BUTTON));
                                mvprintw(et.y, et.x, "0");
                                attroff(COLOR_PAIR(COLORS::BUTTON));
                            }
                        }
                    }

                    if (collides(map.at(render_ray.y).at(render_ray.x), render_ray.y, render_ray.x) && (!debug)) {
                        break;
                    }

                    rays = {};
                    float distance = 0.0;
                    float eye_x = sin(angle);
                    float eye_y = cos(angle);

                    while (distance < depth) {
                        VCVec2 ray;
                        ray.x = (int)player.x + eye_x * distance;
                        ray.y = (int)player.y + eye_y * distance;
                        ray.x = clamp(ray.x, 1, map.at(0).length());
                        ray.y = clamp(ray.y, 1, map.size() - 2);
                        if (map.at(ray.y).at(ray.x) == '#') {
                            break;
                        }

                        rays.push_back(ray);

                        attron(COLOR_PAIR(COLORS::AIM_RAY));
                        mvprintw(ray.y, ray.x, ",");
                        attroff(COLOR_PAIR(COLORS::AIM_RAY));
                        distance += 0.1;
                    }

                    ray_entities = {};

                    int i = 0;
                    for (Entity& entity : entity_list) {
                        VCVec2 entity_pos;
                        entity_pos.x = entity.x;
                        entity_pos.y = entity.y;

                        aiming = false;

                        for (VCVec2& pos : rays) {
                            if (pos.x == entity_pos.x && (pos.y == entity_pos.y) or (silent_aim)) {
                                aiming = true;
                                ray_entities.push_back(&entity);
                                break;
                            }
                        }

                        if (aiming) attron(COLOR_PAIR(COLORS::AIM_RAY));

                        if (entity.entity_id == 0) {
                            if (entity.x == render_ray.x && (entity.y == render_ray.y)) {
                                mvprintw(entity.y, entity.x, "K");
                            }
                        }

                        if (aiming) attroff(COLOR_PAIR(COLORS::AIM_RAY));


                        if (!map.empty() && aiming) {
                            mvprintw(i, ((int)map.at(0).length()) + 1, "Entity %d: %d%% %s", i + 1, entity.health, health_bar(entity.health).c_str());
                        }
                        i++;
                    }

                    ray_entitytiles = {};
                    for (EntityTile& et : entitytile_list) {
                        for (VCVec2& pos : rays) {
                            if (pos.x == et.x && (pos.y == et.y) or (silent_aim)) {
                                ray_entitytiles.push_back(&et);
                            }
                        }
                    }

                    for (Item& item : item_list) {
                        if (item.x == render_ray.x && (item.y == render_ray.y)) {
                            attron(COLOR_PAIR(COLORS::ITEM));
                            mvprintw(item.y, item.x, "%c", ItemTexts::get_item_char(item.id));
                            attroff(COLOR_PAIR(COLORS::ITEM));
                        }
                        move(player.y, player.x);
                    }


                    render_distance += 0.5;
                }
                render_angle += 0.2;
            }

            mvprintw(max_y - 1, 0, "Health: %d%% %s || Ammo: %d Y: %d X: %d", player.health, health_bar(player.health).c_str(), player.ammo, player.y, player.x);
            for (int i = 0; i < player.inventory.size(); i++) {
                int item = player.inventory.at(i);
                mvprintw(max_y - 2 - i, 0, "  %s", ItemTexts::get_item_name(item).c_str());
                if (selected_item == i) mvprintw(max_y - 2 - i, 0, "> %s", ItemTexts::get_item_name(item).c_str());
            }
            mvprintw(player.y, player.x, "@");
            move(player.y, player.x);
        }
        else if (scene == 2) {
            mvprintw(0, 0, "You died!");
            mvprintw(1, 0, "Restart the game to continue from last save");
        }

        int k = getch();
        if (k == 'q') {
            end();
            exit(0);
        }
        else if (k == KEY_DOWN) {
            menu_selection += 1;
        }
        else if (k == KEY_UP) {
            menu_selection -= 1;
        }
        else if (k == '\n') {
            if (scene == 0) {
                if (menu_selection == 0) {
                    attron(COLOR_PAIR(COLORS::BUTTON_CLICK));
                    mvprintw(1, (max_x / 2 - menu_play.length() / 2), menu_play.c_str());
                    mvprintw(1, (max_x / 2 - menu_play.length() / 2), "");
                    attroff(COLOR_PAIR(COLORS::BUTTON_CLICK));
                    refresh();
                    Sleep(100);
                    scene = 1;
                }
                else if (menu_selection == 1) {
                    attron(COLOR_PAIR(COLORS::BUTTON_CLICK));
                    mvprintw(2, (max_x / 2 - menu_exit.length() / 2), menu_exit.c_str());
                    mvprintw(2, (max_x / 2 - menu_exit.length() / 2), "");
                    attroff(COLOR_PAIR(COLORS::BUTTON_CLICK));
                    refresh();
                    Sleep(200);
                    end();
                    exit(0);
                }
            }
            else if (scene == 1) {
                VCVec2 up_tile_coord;
                up_tile_coord.x = player.x;
                up_tile_coord.y = clamp(player.y - 1, 0, map.size() - 1);
                VCVec2 down_tile_coord;
                down_tile_coord.x = player.x;
                down_tile_coord.y = clamp(player.y + 1, 0, map.size() - 1);
                VCVec2 left_tile_coord;
                left_tile_coord.y = player.y;
                left_tile_coord.x = clamp(player.x - 1, 0, map.size() - 1);
                VCVec2 right_tile_coord;
                right_tile_coord.y = player.y;
                right_tile_coord.x = clamp(player.x + 1, 0, map.size() - 1);

                for (Door& door : door_list) {
                    if (up_tile_coord.x == door.x && (up_tile_coord.y == door.y)) {
                        door.opened = !door.opened;
                    }
                    else if (down_tile_coord.x == door.x && (down_tile_coord.y == door.y)) {
                        door.opened = !door.opened;
                    }
                    else if (left_tile_coord.x == door.x && (left_tile_coord.y == door.y)) {
                        door.opened = !door.opened;
                    }
                    else if (right_tile_coord.x == door.x && (right_tile_coord.y == door.y)) {
                        door.opened = !door.opened;
                    }
                }
            }
        }


        if (scene == 1) {
            if (k == 'w') {
                player.y -= 1;
                if (collides(map.at(player.y).at(player.x), player.y, player.x)) {
                    player.y += 1;
                }
            }
            if (k == 's') {
                player.y += 1;
                if (collides(map.at(player.y).at(player.x), player.y, player.x)) {
                    player.y -= 1;
                }
            }
            if (k == 'a') {
                player.x -= 1;
                if (collides(map.at(player.y).at(player.x), player.y, player.x)) {
                    player.x += 1;
                }
                last_move = 0;
            }
            if (k == 'd') {
                player.x += 1;
                if (collides(map.at(player.y).at(player.x), player.y, player.x)) {
                    player.x -= 1;
                }
                last_move = 1;
            }
            if (k == 'z') {
                if (!player.inventory.empty()) {
                    int current_item = player.inventory.at(selected_item);
                    if (current_item == 1 or (current_item == 4)) {
                        if (player.ammo > 0) {
                            player.ammo -= 1;
                            if (!ray_entitytiles.empty()) {
                                if (ray_entitytiles.at(0)->id == 1) {
                                    if (in_range(player.x, ray_entitytiles.at(0)->x - 2, ray_entitytiles.at(0)->x + 2)) {
                                        if (in_range(player.y, ray_entitytiles.at(0)->y - 2, ray_entitytiles.at(0)->y + 2)) {
                                            player.dhealth -= 15;
                                            int old_px = player.x;
                                            int old_py = player.y;
                                            if (player.x < ray_entitytiles.at(0)->x) {
                                                player.x -= 1;
                                                if (collides(map.at(player.y).at(player.x), player.y, player.x)) {
                                                    player.x = old_px;
                                                }
                                            }
                                            else if (player.x > ray_entitytiles.at(0)->x) {
                                                player.x += 1;
                                                if (collides(map.at(player.y).at(player.x), player.y, player.x)) {
                                                    player.x = old_px;
                                                }
                                            }

                                            if (player.y < ray_entitytiles.at(0)->y) {
                                                player.y -= 1;
                                                if (collides(map.at(player.y).at(player.x), player.y, player.x)) {
                                                    player.y = old_py;
                                                }
                                            }
                                            else if (player.y > ray_entitytiles.at(0)->y) {
                                                player.y += 1;
                                                if (collides(map.at(player.y).at(player.x), player.y, player.x)) {
                                                    player.y = old_py;
                                                }
                                            }


                                        }
                                    }
                                    ray_entitytiles.at(0)->health = 0;
                                }
                            }

                            if (!ray_entities.empty()) { // shoot
                                if (current_item == 1) {
                                    ray_entities.at(0)->health -= 15;


                                }
                                else if (current_item == 4) {
                                    int i = 30;
                                    for (Entity* e : ray_entities) {
                                        e->health -= i;
                                        i -= 2;
                                    }
                                }
                            }
                        }
                    }

                    if (current_item == 2) {
                        player.dhealth = clamp(player.health + 15, 0, 100);
                        player.inventory.erase(player.inventory.begin() + selected_item);
                    }

                    if (current_item == 3) {
                        player.ammo += 30;
                        player.inventory.erase(player.inventory.begin() + selected_item);
                    }
                }
            }
            if (k == 'c') {
                save_json();
            }
            if (k == 'i' && (debug)) {
                init_level();
            }
            if (k == KEY_RIGHT) {
                angle += aim_sensivity;
            }
            if (k == KEY_LEFT) {
                angle -= aim_sensivity;
            }
            if (k == KEY_UP) {
                selected_item++;
            }
            if (k == KEY_DOWN) {
                selected_item--;
            }
            if (k == 'k') {
                render_range -= 0.1;
            }
            if (k == 'l') {
                render_range += 1;
            }
            if (k == 'f') {
                player.dashing = true;
                player.dash_count = 5;
            }
            selected_item = clamp(selected_item, 0, player.inventory.size() - 1);
        }

        refresh();


        nc_getmouse(&ev);
        if (ev.x != 0 && (ev.y != 0)) {
            float m_angle = 0;
            while (m_angle < 16.0) {
                float dist = 0.0;
                float eye_x = sin(m_angle);
                float eye_y = cos(m_angle);
                while (dist < max_y) {
                    int mray_x = (int)player.x + eye_x * dist;
                    int mray_y = (int)player.y + eye_y * dist;

                    if (mray_x == ev.x && (mray_y == ev.y)) {
                        angle = m_angle;
                        break;
                    }
                    dist += 0.2;
                }
                m_angle += 0.2;
            }
        }
    }

    ~VChambers() {

    }

};

int main() {
    VChambers* vchambers = new VChambers;

    vchambers->initalize();
    while (true) {
        vchambers->render();
    }
    vchambers->end();

}