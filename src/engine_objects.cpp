//
// Copyright (c) 2024 cppfx.xyz
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "config.hpp"
#include "engine_objects.hpp"

q3viewer::engine_objects::~engine_objects()
{
	if (engine)
	{
		engine->drop();
		engine = nullptr;
	}
	if (q3viewer_destructor_verbose)
		std::cout << "[c++ destructor] q3viewer::~engine_objects: Engine device is closed!" << std::endl;
}

q3viewer::engine_objects::engine_objects(video::E_DRIVER_TYPE edt__, u32 width__, u32 height__)
{
	engine = api::createDevice(
		edt__,
		core::dimension2du{width__, height__},
		32,
		false,
		true,
		false,
		nullptr
	);
	// It is safe to throw exception in constructor here.
	if (! engine)
		throw std::runtime_error{"Failed to create engine device!"};
	// else

	driver = engine->getVideoDriver();
	scene = engine->getSceneManager();
	fs = engine->getFileSystem();
	cursor = engine->getCursorControl();
	gui = engine->getGUIEnvironment();
	meta_selector = scene->createMetaTriangleSelector();
	fps_camera = scene->addCameraSceneNodeFPS(
		nullptr,
		100.0f,
		1.0f,
		-1,
		q3viewer_keymap_wasd,
		q3viewer_keymap_wasd_size,
		false,
		5.0f,
		false,
		true
	);
	cursor->setVisible(false);	// Hide cursor on fps_camera is added.

}

void q3viewer::engine_objects::run() const
{
	while (engine->run())
	{
		if (engine->isWindowActive())
		{
			driver->beginScene(true, true, video::SColor{0xff32779a});
			scene->drawAll();
			driver->endScene();
		}
		else
		{
			engine->yield();
		}
	}
}

void q3viewer::engine_objects::setup_camera_collision()
{
	scene::ISceneNodeAnimator * collision = scene->createCollisionResponseAnimator(
		meta_selector,
		fps_camera,
		core::vector3df{30, 50, 30},
		core::vector3df{0, q3viewer_global_gravity, 0},
		core::vector3df{0, 0, 0},
		0.0001f
	);
	fps_camera->addAnimator(collision);
	collision->drop();
}

