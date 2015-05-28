#include <ssengine/log.h>
#include <lua.hpp>

#ifdef WIN32
#include <Windows.h>
#else
#endif

#include <ssengine/render/resources.h>

#include "ss_drawable.h"
#include <ssengine/render/drawbatch.h>

ss_resource_ref* ss_spine_atlas_resource(ss_core_context* C, const char* uri);
ss_resource_ref* ss_spine_skeleton_resource(ss_core_context* C, const char* _uri);

const ss_resource_type*  ss_spine_skeleton_resource_typetag();

inline ss_spine_drawable* lua_check_drawable(lua_State* L, int idx){
    return (ss_spine_drawable*)luaL_checkudata(L, idx, "spine.Drawable");
}

static int spine_drawable_mt_gc(lua_State *L){
    lua_check_drawable(L, 1)->~ss_spine_drawable();
    return 0;
}

static int spine_drawable_update(lua_State *L){
    lua_check_drawable(L, 1)->update((float)luaL_checknumber(L, 2));
    return 0;
}

static int spine_drawable_draw(lua_State *L){
    lua_check_drawable(L, 1)->draw(ss_lua_check_render2d_context(L, 2));
    return 0;
}


static int lua_create_spine_drawable(lua_State* L){
    struct ss_resource_ref* res = ss_lua_check_resource_ref(L, 1);
    if (res->prototype->typetag != ss_spine_skeleton_resource_typetag()){
        luaL_error(L, "Resource from argument 1 is not type of spineSkeleton.");
    }
    ss_spine_drawable* ptr = reinterpret_cast<ss_spine_drawable*>(lua_newuserdata(L, sizeof(ss_spine_drawable)));
    new(ptr)ss_spine_drawable((spSkeletonData*)(res->ptr), ss_lua_get_core_context(L), res);

    if (luaL_newmetatable(L, "spine.Drawable") == 1){
        luaL_reg resource_methods[] = {
            { "__gc", spine_drawable_mt_gc },
            { "update", spine_drawable_update },
            { "draw", spine_drawable_draw },
            { NULL, NULL }
        };

        for (size_t i = 0; resource_methods[i].func != nullptr; ++i){
            lua_pushcfunction(L, resource_methods[i].func);
            lua_setfield(L, -2, resource_methods[i].name);
        }
        // set self as "__index" metamethod.
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);
    return 1;
}

extern "C" SS_EXPORT int luaopen_spine_native(lua_State*L);

int luaopen_spine_native(lua_State*L){
    luaL_Reg funcs[] = {
        { "createDrawable", lua_create_spine_drawable },
        { NULL, NULL }
};
#if LUA_VERSION_NUM >= 502
    luaL_newlib(L, funcs);
#else
    luaL_register(L, "spine.native", funcs);
#endif

    ss_lua_require_module(L, "resource");
    lua_getfield(L, -1, "loaders");
    {
        // Initialize resource loaders.
        lua_pushlightuserdata(L, (resource_loader_func)ss_spine_atlas_resource);
        lua_setfield(L, -2, "spineAtlas");

        lua_pushlightuserdata(L, (resource_loader_func)ss_spine_skeleton_resource);
        lua_setfield(L, -2, "spineSkeleton");
    }

    lua_pop(L, 2);
    return 1;
}