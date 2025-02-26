//
// Copyright (c) 2024 cppfx.xyz
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "q3_map_loader.hpp"
#include <filesystem>
#include <string_view>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

namespace q3viewer
{

class qm_index_to_string_class
{
public:
	std::string operator()(const quake3::eQ3MeshIndex qm_index) const
	{
		switch (qm_index)
		{
		case quake3::E_Q3_MESH_GEOMETRY:
			return "api::scene::quake3::E_Q3_MESH_GEOMETRY";
		case quake3::E_Q3_MESH_ITEMS:
			return "api::scene::quake3::E_Q3_MESH_ITEMS";
		case quake3::E_Q3_MESH_BILLBOARD:
			return "api::scene::quake3::E_Q3_MESH_BILLBOARD";
		case quake3::E_Q3_MESH_FOG:
			return "api::scene::quake3::E_Q3_MESH_FOG";
		case quake3::E_Q3_MESH_UNRESOLVED:
			return "api::scene::quake3::E_Q3_MESH_UNRESOLVED";
		case quake3::E_Q3_MESH_SIZE:
			return "api::scene::quake3::E_Q3_MESH_SIZE";
		}
		return "Unknown Shader";
	}
};

class qm_index_to_wstring_class
{
public:
	std::wstring operator()(const quake3::eQ3MeshIndex qm_index) const
	{
		switch (qm_index)
		{
		case quake3::E_Q3_MESH_GEOMETRY:
			return L"api::scene::quake3::E_Q3_MESH_GEOMETRY";
		case quake3::E_Q3_MESH_ITEMS:
			return L"api::scene::quake3::E_Q3_MESH_ITEMS";
		case quake3::E_Q3_MESH_BILLBOARD:
			return L"api::scene::quake3::E_Q3_MESH_BILLBOARD";
		case quake3::E_Q3_MESH_FOG:
			return L"api::scene::quake3::E_Q3_MESH_FOG";
		case quake3::E_Q3_MESH_UNRESOLVED:
			return L"api::scene::quake3::E_Q3_MESH_UNRESOLVED";
		case quake3::E_Q3_MESH_SIZE:
			return L"api::scene::quake3::E_Q3_MESH_SIZE";
		}
		return L"";
	}
};

class to_wstring_class
{
public:
	std::wstring operator()(const std::string & str) const
	{
		std::wstring wstr;
		for (const auto & x: str)
		{
			wstr.push_back(static_cast<wchar_t>(x));
		}
		return wstr;
	}
};

}	// namespace q3viewer

q3viewer::q3_map_loader::~q3_map_loader()
{
	if (q3viewer_destructor_verbose)
		std::cout << "[c++ destructor] q3viewer::~q3_map_loader: q3_map_loader is unloaded!\n";
}

q3viewer::q3_map_loader::q3_map_loader(q3viewer::engine_objects & engine__, i32 polys__):
	engine{engine__},
	polys{polys__}
{
	// It is safe to throw exception here in constructor.
	if (! engine.engine)
		throw std::runtime_error{"No engine is created!"};
}

void q3viewer::q3_map_loader::load(const std::string & pk3_name__)
{
	this->load_map(pk3_name__);
	this->load_shaders();
	this->spawn_player();
}

void q3viewer::q3_map_loader::load_map(const std::string & pk3_name__)
{
	if (pk3_name__ == "")
		throw std::runtime_error{"Emtpy pk3 archive name is not allowed!"};
	if (pk3_name__ == "/")
		throw std::runtime_error{"Root path / as pk3 archive name is not allowed!"};

	std::string pk3_name;

	if (pk3_name__ == "." || pk3_name__ == "./" || pk3_name__ == ".//")
		pk3_name = fs::current_path().string();
	else if (pk3_name__ == ".." || pk3_name__ == "../" || pk3_name__ == "..//")
		pk3_name = fs::current_path().parent_path().string();
	else
		pk3_name = pk3_name__;
	
	if (engine.fs->getFileArchiveCount() != 0u)
	{
		std::cout << "ERROR: this should not happen before adding: engine.fs->getFileArchiveCount() != 0u .\n";
		return;
	}
	if (! engine.fs->addFileArchive(pk3_name.data()))
	{
		std::cout << "ERROR: pk3 archive can not be added: " << pk3_name << std::endl;
		return;
	}
	if (engine.fs->getFileArchiveCount() != 1u)
	{
		std::cout << "ERROR: this should not happen after adding: engine.fs->getFileArchiveCount() != 1u .\n";
		return;
	}

	std::string map_name__;
	const api::io::IFileArchive * archive__ = engine.fs->getFileArchive(0u);
	const api::io::IFileList * file_list__ = archive__->getFileList();
	for (u32 i=0; i<file_list__->getFileCount(); ++i)
	{
		const api::io::path & path__ = file_list__->getFileName(i);
		if (std::string_view{path__.data()}.ends_with(".bsp"))
			map_name__ = path__.data();
	}
	if (map_name__.empty() || ! map_name__.ends_with(".bsp"))
	{
		std::cout << "ERROR: can not find map mesh file !" << std::endl;
		return;
	}

	mesh_mv = engine.scene->getMesh(map_name__.data());
	if (! mesh_mv)
		throw std::runtime_error{"bsp map can not be loaded: "s + map_name__};
	std::cout << "Loaded: " << map_name__ << std::endl;
	
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
}

void q3viewer::q3_map_loader::load_shaders()
{
	std::ostringstream cout_buffer;
	cout_buffer << "--------------------------------------------------------------------------------\n"
		<< ":: " << q3viewer::qm_index_to_string_class{}(api::scene::quake3::E_Q3_MESH_SIZE)
		<< " :\t" << quake3::E_Q3_MESH_SIZE
		<< "\n=>" << std::endl;
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
		cout_buffer
			<< "\t:: "
			<< std::left << std::setw(40) << q3viewer::qm_index_to_string_class{}(qm_index)
			<< std::right << std::setw(3) << ":"
			<< std::right << std::setw(10)  << qm_index << std::endl;
		scene::IMesh * e_mesh = mesh_qv->getMesh(qm_index);
		if (! e_mesh)
			continue;
		i32 mb_count = e_mesh->getMeshBufferCount();
		cout_buffer << "\t\t:: Mesh Buffer Count:\t" << mb_count << std::endl;
		api::int32_pub i=0;
		for (; i<mb_count; ++i)
		{
			scene::IMeshBuffer * mesh_buffer = e_mesh->getMeshBuffer(i);
			if (! mesh_buffer)
				continue;
			video::SMaterial & material = mesh_buffer->getMaterial();
			i32 shader_index = (i32)material.MaterialTypeParam2;
			const quake3::IShader * shader = mesh_qv->getShader(shader_index);
			if (! shader)
				continue;
			scene::IMeshSceneNode * node = engine.scene->addQuake3SceneNode(
				mesh_buffer,
				shader,
				nullptr,
				-1
			);
			if (! node)
			{
				std::cout << "::::::::::: invalid node ! ::::::::::::\n";
				continue;
			}
			scene::ITextSceneNode * text_node = engine.scene->addTextSceneNode(
				engine.gui->getSkin()->getFont(),
				q3viewer::qm_index_to_wstring_class{}(qm_index).data(),
				video::SColor{0xff32ff32},
				node,
				nub::vector3df{0},
				-1
			);
			switch (qm_index)
			{
			case quake3::E_Q3_MESH_FOG:
				text_node->setText((L"Fog: "s + q3viewer::to_wstring_class{}(shader->name.data())).data());
				break;
			case quake3::E_Q3_MESH_ITEMS:
				text_node->setText((L"Item: "s + q3viewer::to_wstring_class{}(shader->name.data())).data());
				break;
			default:
				break;
			}
			scene::ITriangleSelector * selector = engine.scene->createTriangleSelector(
				node->getMesh(),
				node
				//false		// for 1.9
			);
			node->setTriangleSelector(selector);
			engine.meta_selector->addTriangleSelector(selector);
			selector->drop();
		}
		if (i == 0)
			cout_buffer << "\t\t\t:: No Shader !" << std::endl;
		else
			cout_buffer << "\t\t\t:: Loaded Shaders :\t" << i << std::endl;
	}
	cout_buffer << "--------------------------------------------------------------------------------\n";

	{
		std::unique_lock<std::mutex> lock{q3viewer_print_mutex};
		std::cout << cout_buffer.str();
	}
}

void q3viewer::q3_map_loader::spawn_player()
{
	quake3::tQ3EntityList & elist = mesh_qv->getEntityList();
	
	quake3::IEntity search;
	search.name = "info_player_deathmatch";
	
	i32 result{-1};
	{
		i32 rd = q3viewer_random()%10;
		i32 npos{0};
		for (i32 i=0; i<rd; ++i)
		{
			i32 result2 = elist.binary_search_multi(search, npos);
			if (result2 < 0)
				break;
			// else
			result = result2;
		}
	}
	if (result < 0)
	{
		search.name = "info_player_start";
		i32 npos{0};
		result = elist.binary_search_multi(search, npos);
	}
	if (result < 0)
	{
		std::cout << "ERROR: No spawn position for player." << std::endl;
		return;
	}
	
	quake3::IEntity & entity = elist[result];
	
	i32 g_size = entity.getGroupSize();
	
	if (g_size < 2)
	{
		std::cout << "ERROR: g_size = " << g_size << std::endl;
		return;
	}
	
	const quake3::SVarGroup * group = entity.getGroup(1);
	
	{
		nub::string pos_str = group->get("origin");
		nub::string angle_str = group->get("angle");
		u32 npos{0};
		nub::vector3df pos = quake3::getAsVector3df(pos_str, npos);
		npos = 0;
		f32 angle = quake3::getAsFloat(angle_str, npos);
		engine.fps_camera->setPosition(pos);
		nub::vector3df dir{0, 0, 1};
		dir.rotateXZBy(angle, nub::vector3df{0,1,0});
		engine.fps_camera->setTarget(pos+dir);
		engine.fps_camera->setFarValue(100000);
		std::cout << "fps_camera position set: (" << pos.X << ", " << pos.Y << ", " << pos.Z << ")\n";
	}
}

