//
// Copyright (c) 2024 cppfx.xyz
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __cppfx_q3_map_loader_hpp__
#define __cppfx_q3_map_loader_hpp__

#include "config.hpp"
#include "engine_objects.hpp"

namespace q3viewer
{

class q3_map_loader
{
protected:
	q3viewer::engine_objects & engine;	// Attach external object
protected:
	scene::IAnimatedMesh * mesh_mv;	// model viewer of mesh
	scene::IQ3LevelMesh * mesh_qv;	// quake3 viewer of mesh
	s32 polys;
public:
	virtual ~q3_map_loader();
public:
	q3_map_loader(q3viewer::engine_objects & engine__, s32 polys__);
public:
	void load(const std::string & pk3_name);
protected:
	void load_map(const std::string & pk3_name__);
	void load_shaders();
	void spawn_player();
};

}	// namespace q3viewer

#endif	// __cppfx_q3_map_loader_hpp__

