#include <spine/extension.h>

#include <ssengine/uri.h>
#include <ssengine/render/resources.h>
#include <ssengine/render/types.h>

//TODO: support async load (while ss_texture2d_resource cannot be invoked from main thread)
void _spAtlasPage_createTexture(ss_core_context* C, spAtlasPage* self, const char* path){
    ss_texture2d_resource_ref* ref = ss_texture2d_resource_ref::get(C, path);
    if (ss_resource_load(C, ref->unwrap()) != 0){
        return;
    }
    self->rendererObject = ref;
}

void _spAtlasPage_disposeTexture(ss_core_context* C, spAtlasPage* self){
    if (self->rendererObject != nullptr){
        ss_resource_release(C, (ss_resource_ref*)self->rendererObject);
        self->rendererObject = nullptr;
    }
}

char* _spUtil_readFile(ss_core_context* C, const char* path, int* length){
    input_stream* is = ss_uri_open_for_read(C, path);

    if (!is){
        return nullptr;
    }

    if (is->seek(0, SEEK_END)){
        *length = (int)is->tell();
        is->seek(0, SEEK_SET);
        char* data = (char*)MALLOC(char, *length);
        is->read(data, *length);

        delete is;
        return data;
    }

    std::string buf;
    char tmp[1024];

    while (size_t readlen = is->read(tmp, 1024) >= 0){
        buf.append(tmp, readlen);
    }

    *length = buf.size();

    char* data = (char*)MALLOC(char, buf.size());
    memcpy(data, buf.c_str(), buf.size());
    return data;
}
