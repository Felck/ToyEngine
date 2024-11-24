#pragma once

// IWYU pragma: begin_exports

#include <stdint.h>

// STL
#include <algorithm>
#include <cassert>
#include <concepts>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// IWYU pragma: end_exports

#define LOG(x) std::cout << x << std::endl