#ifndef _SHARED_DATA_HPP
#define _SHARED_DATA_HPP

#pragma once

#include <exception>
#include <string>
#include <map>
#include <memory>
#include <utility>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <stop_token>

#include "head.hpp"

namespace shared_data {

std::stop_source sts_ui_;

pt::ProcessThread pt_;

}

#endif // !_SHARED_DATA_HPP
