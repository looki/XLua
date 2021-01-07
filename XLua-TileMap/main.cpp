#define SOL_CHECK_ARGUMENTS
#define SOL_SAFE_USERTYPE

#include "sol.hpp"
#include "xlua.h"

#include "Common.h"
#include "Rundata.h"

#include <sstream>

static lua_State * L = nullptr;


namespace sol {
	inline void assert_throw(bool assertion, const char * message) {
		if (!assertion) {
			luaL_traceback(L, L, message, 0);
			const char * error_string = lua_tostring(L, -1);
			lua_pop(L, 1);
			throw sol::error(error_string);
		}
	}
}

struct XLayer {
	TILEMAP * tile_map;
	Layer * layer;

	XLayer(TILEMAP * tile_map) : tile_map(tile_map) {
		layer = tile_map->currentLayer;
		sol::assert_throw(layer != nullptr, "No layer selected");
	}

	XLayer(TILEMAP * tile_map, size_t i) : tile_map(tile_map) {
		layer = tile_map->getLayerAt(tile_map, i);
		sol::assert_throw(layer != nullptr, "No layer selected");
	}

	inline void assert_coord(uint16_t x, uint16_t y) {
		sol::assert_throw(x < layer->getWidth() && y < layer->getHeight(),
			"Tile index out of bounds");
	}

	void set(uint16_t x, uint16_t y, uint8_t tile_x, uint8_t tile_y) {
		assert_coord(x, y);
		Tile * tile = layer->getTile(x, y);
		tile->x = tile_x;
		tile->y = tile_y;
	}

	void unset(uint16_t x, uint16_t y) {
		assert_coord(x, y);
		*layer->getTile(x, y) = Tile::ByID(Tile::EMPTY);
	}

	void fill(uint8_t tile_x, uint8_t tile_y) {
		Tile * data = layer->getTile(0, 0);
		std::fill(data, data + width() * height(), Tile::ByXY(tile_x, tile_y));
	}

	std::tuple<uint8_t, uint8_t> get(uint16_t x, uint16_t y) {
		assert_coord(x, y);
		Tile * tile = layer->getTile(x, y);
		return std::make_tuple(tile->x, tile->y);
	}

	uint8_t get_x(uint16_t x, uint16_t y) {
		assert_coord(x, y);
		return layer->getTile(x, y)->x;
	}

	uint8_t get_y(uint16_t x, uint16_t y) {
		assert_coord(x, y);
		return layer->getTile(x, y)->y;
	}

	void for_each(const sol::function & callback) {
		uint8_t tile_x, tile_y;
		for (size_t x = 0; x < width(); ++x) {
			for (size_t y = 0; y < height(); ++y) {
				Tile * tile = layer->getTile(x, y);
				sol::tie(tile_x, tile_y) = callback(x, y);
				tile->x = tile_x;
				tile->y = tile_y;
			}
		}
	}

	size_t width() const {
		return layer->getWidth();
	}

	size_t height() const {
		return layer->getHeight();
	}

	size_t index() const {
		return layer - tile_map->getLayerAt(tile_map, 0);
	}

	operator std::string() const {
		std::ostringstream stream;
		stream << "Layer " << index() << " (" << width() << "x" << height() << ")";
		return std::move(stream.str());
	}
};

struct XTileMap {
	TILEMAP * impl;

	XTileMap(uint16_t fixed_value) {
		LPRH rh = xlua_get_run_header(L);
		LPOBL obj_list = rh->rhObjectList;
		impl = reinterpret_cast<TILEMAP *>(obj_list[fixed_value].oblOffset);
		sol::assert_throw(impl != nullptr && impl->rHo.hoIdentifier == MAKEID(T, M, A, P),
			"Fixed value does not correspond to a Tile Map instance");
	}

	int layer_count() const {
		return impl->getLayerCount(impl);
	}

	XLayer layer_at(int i) {
		return XLayer(impl, i);
	}

	XLayer get_current_layer() {
		return XLayer(impl);
	}

	void set_current_layer(const XLayer & layer) {
		impl->currentLayer = layer.layer;
	}

	operator std::string() const {
		std::ostringstream stream;
		int fixed_value = ((impl->rHo.hoCreationId << 16) + impl->rHo.hoNumber);
		stream << "TileMap " << fixed_value;
		return std::move(stream.str());
	}
};

extern "C" __declspec(dllexport) int luaopen_tilemap(lua_State * state) {
	L = state;
	sol::state_view lua(L);

	lua.new_usertype<XTileMap>("TileMap",
		sol::constructors<sol::types<uint16_t>>(),
		"current_layer", sol::property(&XTileMap::get_current_layer, &XTileMap::set_current_layer),
		sol::meta_method::length, &XTileMap::layer_count,
		sol::meta_method::index, &XTileMap::layer_at,
		sol::meta_method::to_string, &XTileMap::operator std::string);

	lua.new_usertype<XLayer>("Layer",
		"index", sol::readonly_property(&XLayer::index),
		"width", sol::readonly_property(&XLayer::width),
		"height", sol::readonly_property(&XLayer::height),
		"set", &XLayer::set,
		"unset", &XLayer::unset,
		"get", &XLayer::get,
		"get_x", &XLayer::get_x,
		"get_y", &XLayer::get_y,
		"fill", &XLayer::fill,
		"for_each", &XLayer::for_each,
		sol::meta_method::to_string, &XLayer::operator std::string);

	return 0;
}
