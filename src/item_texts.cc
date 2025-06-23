#include "item_texts.hh"

char vc::ItemTexts::get_item_char(int id) {
	switch (id) {
	case 1:
		return '/';
	case 2:
		return '+';
	case 3:
		return '|';
	case 4:
		return '*';
	}
	return std::to_string(id).at(0);
}

std::string vc::ItemTexts::get_item_name(int id) {
	switch (id) {
	case 1:
		return "Piston";
	case 2:
		return "Medkit";
	case 3:
		return "Ammo";
	case 4:
		return "Shotgun";
	}
	return "ItemId." + std::to_string(id);
}
