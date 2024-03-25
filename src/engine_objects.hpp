//
// Copyright (c) 2024 cppfx.xyz
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __cppfx_engine_objects_hpp__
#define __cppfx_engine_objects_hpp__
#include "config.hpp"

namespace q3viewer
{

class engine_objects
{
public:
	q3viewer::game_engine * engine;
	video::IVideoDriver * driver;
	scene::ISceneManager * scene;
	io::IFileSystem * fs;
	gui::ICursorControl * cursor;
	gui::IGUIEnvironment * gui;
	scene::IMetaTriangleSelector * meta_selector;
	scene::ICameraSceneNode * fps_camera;

public:
	virtual ~engine_objects();
	engine_objects(video::E_DRIVER_TYPE edt__, u32 width__, u32 height__);

public:
	void run() const;
	void setup_camera_collision();
};	// class engine_objects

}	// namespace q3viewer

#endif	// __cppfx_engine_objects_hpp__

