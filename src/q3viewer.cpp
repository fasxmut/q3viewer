//
// Copyright (c) 2024 cppfx.xyz
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "config.hpp"
#include "engine_objects.hpp"
#include "q3_map_loader.hpp"

int main(int argc, char * argv[])
try
{
	if (argc != 3)
	{
		std::cout << "q3viewer <pk3 archive> <bsp name>" << std::endl;
		return 0;
	}
	q3viewer::engine_objects engine{video::EDT_OPENGL, 1280, 720};
	engine.engine->setWindowCaption(L"q3viewer - Quake3 Map Level Viewer");
	q3viewer::q3_map_loader qm_loader{engine, 1024};
	qm_loader.load(argv[1], argv[2]);
	
	engine.setup_camera_collision();
	engine.run();
}
catch (const std::exception & err)
{
	std::cerr << "Main [std::exception]:\n" << err.what() << std::endl;
	return 1;
}

