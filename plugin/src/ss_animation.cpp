#include <ssengine/log.h>
#include <ssengine/uri.h>
#include <ssengine/render/resources.h>

#include <spine/SkeletonJson.h>

#include <regex>

static ss_resource_type spine_skeleton_resource_type_tag;

const ss_resource_type*  ss_spine_skeleton_resource_typetag(){
    return &spine_skeleton_resource_type_tag;
}

ss_resource_ref* ss_spine_atlas_resource(ss_core_context* C, const char* uri);

static int (spine_skeleton_sync_load)(ss_core_context* C, ss_resource_ref* ref){
    std::string _uri = ss_resource_get_uri(C, ref);
    ss_uri uri = ss_uri::parse(C, _uri);
    
    ss_resource_ref* ref_atlas = ss_spine_atlas_resource(C, uri.search.c_str());
    ss_resource_load(C, ref_atlas);

    //uri.search.clear();
    //_uri = uri.str();

    if (ref_atlas->ptr == nullptr){
        // Failed to load atlas;
        return -1;
    }
    
    spAtlas* atlas = (spAtlas*)ref_atlas->ptr;

    spSkeletonJson* json = spSkeletonJson_create(atlas);
    ref->ptr = spSkeletonJson_readSkeletonDataFile(C, json, _uri.c_str());

    ss_resource_release(C, ref_atlas);

    spSkeletonJson_dispose(json);
    return ref->ptr ? 0 : -1;
}

static void (spine_skeleton_unload)(ss_core_context* C, ss_resource_ref* ref){
    if (ref->ptr){
        spSkeletonData_dispose((spSkeletonData*)ref->ptr);
        ref->ptr = nullptr;
    }
}

static ss_resource_prototype  spine_skeleton_prototype = {
    &spine_skeleton_resource_type_tag,
    spine_skeleton_sync_load,
    NULL,
    spine_skeleton_unload
};


ss_resource_ref* ss_spine_skeleton_resource(ss_core_context* C, const char* _uri){
    ss_resource_ref* ret;
    ret = ss_resource_from_uri(C, _uri);
    if (ret){
        return ret;
    }
    ret = ss_resource_create(C, &spine_skeleton_prototype, _uri, SS_DT_SOFTWARE);
    return ret;
}