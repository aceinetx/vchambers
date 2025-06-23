#include "math.hh"
#include <random>
#include <time.h>

bool vc::math::in_range(int value, int _min, int _max) {
	if (value >= _min && (value <= _max))
		return true;
	return false;
}

int vc::math::clamp(int value, int _min, int _max) {
	if (value < _min)
		return _min;
	if (value > _max)
		return _max;
	return value;
}

int vc::math::random(int _min, int _max) {
	int result = _min + (rand() % _max + 1);

	srand((unsigned)time(NULL));

	return result;
}
