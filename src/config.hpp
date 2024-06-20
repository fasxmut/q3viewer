//
// Copyright (c) 2024 cppfx.xyz
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __cppfx_q3viewer_config_hpp__
#define __cppfx_q3viewer_config_hpp__

#include <iostream>
#include <string>
#include <random>

using s32 = api::s32;
using u32 = api::u32;
using f32 = api::f32;

using std::operator""s;

namespace q3viewer
{
using game_engine = engine_device;
}	// namespace q3viewer

namespace video = api::video;
namespace scene = api::scene;
namespace io = api::io;
namespace gui = api::gui;
namespace core = api::core;
namespace quake3 = scene::quake3;

constexpr bool q3viewer_destructor_verbose = true;	// Print verbose information in all destructors.
constexpr f32 q3viewer_global_gravity = 0;
inline std::mt19937 q3viewer_random{std::random_device{}()};

inline api::SKeyMap q3viewer_keymap_wasd[] =
	{
		{api::EKA_MOVE_FORWARD, api::KEY_KEY_W},
		{api::EKA_MOVE_BACKWARD, api::KEY_KEY_S},
		{api::EKA_STRAFE_LEFT, api::KEY_KEY_A},
		{api::EKA_STRAFE_RIGHT, api::KEY_KEY_D}
	};
inline s32 q3viewer_keymap_wasd_size = 4;

#endif	// __cppfx_q3viewer_config_hpp__

