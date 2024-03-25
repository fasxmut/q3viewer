//
// Copyright (c) 2024 cppfx.xyz
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "q3_map_loader.hpp"
#include <filesystem>

namespace fs = std::filesystem;

q3viewer::q3_map_loader::~q3_map_loader()
{
	if (q3viewer_destructor_verbose)
		std::cout << "[c++ destructor] q3viewer::~q3_map_loader: q3_map_loader is unloaded!\n";
}

q3viewer::q3_map_loader::q3_map_loader(q3viewer::engine_objects & engine__, s32 polys__):
	engine{engine__},
	polys{polys__}
{
	// It is safe to throw exception here in constructor.
	if (! engine.engine)
		throw std::runtime_error{"No engine is created!"};
}

void q3viewer::q3_map_loader::load(const std::string & pk3_name__, const std::string & bsp_name)
{
	if (pk3_name__ == "")
		throw std::runtime_error{"Emtpy pk3 archive name is not allowed!"};
	if (pk3_name__ == "/")
		throw std::runtime_error{"Root path / as pk3 archive name is not allowed!"};
	if (bsp_name == "")
		throw std::runtime_error{"Emtpy bsp name is not allowed!"};
		
	std::string pk3_name;
	
	if (pk3_name__ == "." || pk3_name__ == "./" || pk3_name__ == ".//")
		pk3_name = fs::current_path().string();
	else if (pk3_name__ == ".." || pk3_name__ == "../" || pk3_name__ == "..//")
		pk3_name = fs::current_path().parent_path().string();
	else
		pk3_name = pk3_name__;
	
	if (! engine.fs->addFileArchive(pk3_name.data()))
	{
		std::cout << "pk3 archive can not be added: " << pk3_name << std::endl;
		return;
	}
	mesh_mv = engine.scene->getMesh(bsp_name.data());
	if (! mesh_mv)
		throw std::runtime_error{"bsp map can not be loaded: "s + bsp_name};
	std::cout << "Loaded: " << bsp_name << std::endl;
	
	mesh_qv = static_cast<scene::IQ3LevelMesh *>(mesh_mv);
	
	scene::IMesh * g_mesh = mesh_qv->getMesh(quake3::E_Q3_MESH_GEOMETRY);
	/*
		1.8:
			scene::IMeshSceneNode
		1.9:
			scene::IOctreeSceneNode
	*/
	auto * g_node = engine.scene->addOctreeSceneNode(
		g_mesh,
		nullptr,
		-1,
		polys
	);
	
	scene::ITriangleSelector * g_selector = engine.scene->createOctreeTriangleSelector(
		g_mesh,
		g_node,
		polys
	);
	g_node->setTriangleSelector(g_selector);
	engine.meta_selector->addTriangleSelector(g_selector);
	g_selector->drop();
	
	this->load_shaders();
}

void q3viewer::q3_map_loader::load_shaders()
{
	std::cout << "====\n" << quake3::E_Q3_MESH_SIZE << std::endl;
	for (
		quake3::eQ3MeshIndex qm_index:
			{
				quake3::E_Q3_MESH_ITEMS,
				quake3::E_Q3_MESH_BILLBOARD,
				quake3::E_Q3_MESH_FOG,
				quake3::E_Q3_MESH_UNRESOLVED
			}
	)
	{
		std::cout << "\t" << qm_index << std::endl;
		scene::IMesh * e_mesh = mesh_qv->getMesh(qm_index);
		if (! e_mesh)
			continue;
		s32 mb_count = e_mesh->getMeshBufferCount();
		std::cout << "\t\t" << mb_count << std::endl;
		for (s32 i=0; i<mb_count; ++i)
		{
			scene::IMeshBuffer * mesh_buffer = e_mesh->getMeshBuffer(i);
			if (! mesh_buffer)
				continue;
			video::SMaterial & material = mesh_buffer->getMaterial();
			s32 shader_index = (s32)material.MaterialTypeParam2;
			const quake3::IShader * shader = mesh_qv->getShader(shader_index);
			if (! shader)
				continue;
			scene::IMeshSceneNode * node = engine.scene->addQuake3SceneNode(
				mesh_buffer,
				shader,
				nullptr,
				-1
			);
			scene::ITriangleSelector * selector = engine.scene->createTriangleSelector(
				node->getMesh(),
				node
				//false		// for 1.9
			);
			node->setTriangleSelector(selector);
			engine.meta_selector->addTriangleSelector(selector);
			selector->drop();
		}
	}
	this->spawn_player();
}

void q3viewer::q3_map_loader::spawn_player()
{
	quake3::tQ3EntityList & elist = mesh_qv->getEntityList();
	
	quake3::IEntity search;
	search.name = "info_player_deathmatch";
	
	s32 result{-1};
	{
		s32 rd = q3viewer_random()%10;
		s32 npos{0};
		for (s32 i=0; i<rd; ++i)
		{
			s32 result2 = elist.binary_search_multi(search, npos);
			if (result2 < 0)
				break;
			// else
			result = result2;
		}
	}
	if (result < 0)
	{
		search.name = "info_player_start";
		s32 npos{0};
		result = elist.binary_search_multi(search, npos);
	}
	if (result < 0)
	{
		std::cout << "ERROR: No spawn position for player." << std::endl;
		return;
	}
	
	quake3::IEntity & entity = elist[result];
	
	s32 g_size = entity.getGroupSize();
	
	if (g_size < 2)
	{
		std::cout << "ERROR: g_size = " << g_size << std::endl;
		return;
	}
	
	const quake3::SVarGroup * group = entity.getGroup(1);
	
	{
		core::stringc pos_str = group->get("origin");
		core::stringc angle_str = group->get("angle");
		u32 npos{0};
		core::vector3df pos = quake3::getAsVector3df(pos_str, npos);
		npos = 0;
		f32 angle = quake3::getAsFloat(angle_str, npos);
		engine.fps_camera->setPosition(pos);
		core::vector3df dir{0, 0, 1};
		dir.rotateXZBy(angle, core::vector3df{0,1,0});
		engine.fps_camera->setTarget(pos+dir);
		engine.fps_camera->setFarValue(100000);
		std::cout << "fps_camera position set: (" << pos.X << ", " << pos.Y << ", " << pos.Z << ")\n";
	}
}

