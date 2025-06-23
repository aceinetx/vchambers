#pragma once

namespace vc {
namespace math {
bool in_range(int value, int _min, int _max);
int clamp(int value, int _min, int _max);
int random(int _min, int _max);
} // namespace math
} // namespace vc
