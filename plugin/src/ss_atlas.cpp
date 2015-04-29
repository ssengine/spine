#include <ssengine/log.h>
#include <ssengine/uri.h>
#include <ssengine/render/resources.h>

#include <spine/Atlas.h>

static ss_resource_type spine_atlas_resource_type_tag;

const ss_resource_type*  ss_spine_atlas_resource_typetag(){
    return &spine_atlas_resource_type_tag;
}

static int (spine_atlas_sync_load)(ss_core_context* C, ss_resource_ref* ref){
    std::string uri = ss_resource_get_uri(C, ref);

    ref->ptr = spAtlas_createFromFile(C, uri.c_str(), nullptr);
    return ref->ptr ? 0 : -1;
}

static void (spine_atlas_unload)(ss_core_context* C, ss_resource_ref* ref){
    if (ref->ptr){
        spAtlas_dispose(C, (spAtlas*)ref->ptr);
        ref->ptr = nullptr;
    }
}

static ss_resource_prototype  spine_atlas_prototype = {
    &spine_atlas_resource_type_tag,
    spine_atlas_sync_load,
    NULL,
    spine_atlas_unload
};

ss_resource_ref* ss_spine_atlas_resource(ss_core_context* C, const char* uri){
    ss_resource_ref* ret;
    ret = ss_resource_from_uri(C, uri);
    if (ret){
        return ret;
    }
    ret = ss_resource_create(C, &spine_atlas_prototype, uri, SS_DT_SOFTWARE);
    return ret;
}