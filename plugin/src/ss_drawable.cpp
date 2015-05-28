#include "ss_drawable.h"

#include <stdint.h>
#include <ssengine/render/resources.h>
#include <ssengine/render/drawbatch.h>

#include <spine/Bone.h>
#include <spine/SlotData.h>
#include <spine/RegionAttachment.h>
#include <spine/MeshAttachment.h>
#include <spine/SkinnedMeshAttachment.h>

#define MAX_VERTEX_COUNT (1024)

ss_spine_drawable::ss_spine_drawable(spSkeletonData* skeletonData, ss_core_context* C, ss_resource_ref* res)
    : timeScale(1), res_(res), C_(C)
{
    spBone_setYDown(true);

    skeleton = spSkeleton_create(skeletonData);
    spAnimationStateData* stateData = spAnimationStateData_create(skeletonData);
    state = spAnimationState_create(stateData);

    ss_resource_addref(C, res);
}

ss_spine_drawable::~ss_spine_drawable()
{
    spAnimationStateData_dispose(state->data);
    spAnimationState_dispose(state);
    spSkeleton_dispose(skeleton);
    ss_resource_release(C_, res_);
}

void ss_spine_drawable::update(float deltaTime) {
    spSkeleton_update(skeleton, deltaTime);
    spAnimationState_update(state, deltaTime * timeScale);
    spAnimationState_apply(state, skeleton);
    spSkeleton_updateWorldTransform(skeleton);
}

//TODO: use index buffer to reduce memory usage and improve performance.

void ss_spine_drawable::draw(render2d_context* RC) const{
    ss_core_context* C = C_;
    ss_draw_batch* db = ss_core_get_draw_batch(C);

    const ss_matrix& m = RC->matrix_stack.top();

    float worldVertices[MAX_VERTEX_COUNT * 2];

    for (int i = 0; i < skeleton->slotsCount; ++i) {
        spSlot* slot = skeleton->drawOrder[i];
        spAttachment* attachment = slot->attachment;
        if (!attachment) continue;

        switch (slot->data->blendMode) {
        case SP_BLEND_MODE_ADDITIVE:
        case SP_BLEND_MODE_MULTIPLY:
        case SP_BLEND_MODE_SCREEN: // Unsupported, fall through.
        default:
            break;
        }
        
        ss_texture2d_resource_ref* ref_texture = 0;

        if (attachment->type == SP_ATTACHMENT_REGION) {
            spRegionAttachment* regionAttachment = (spRegionAttachment*)attachment;
            ref_texture = (ss_texture2d_resource_ref*)((spAtlasRegion*)regionAttachment->rendererObject)->page->rendererObject;

            ss_texture2d* texture = ref_texture -> get();
            if (texture == nullptr){
                continue;
            }
            

            spRegionAttachment_computeWorldVertices(regionAttachment, slot->bone, worldVertices);

            db->prepare(texture, SS_PT_TRIANGLELIST, 6);

            db->diffuse(0) = db->diffuse(1) =
                db->diffuse(2) = db->diffuse(3) =
                db->diffuse(4) = db->diffuse(5) = ss::color(
                    skeleton->r * slot->r, 
                    skeleton->g * slot->g, 
                    skeleton->b * slot->b, 
                    skeleton->a * slot->a);

            db->pos(0) = db->pos(3) = m.transpose(ss::float2(worldVertices[SP_VERTEX_X1], worldVertices[SP_VERTEX_Y1]));
            db->texcoord(0) = db->texcoord(3) = ss::float2(regionAttachment->uvs[SP_VERTEX_X1], regionAttachment->uvs[SP_VERTEX_Y1]);

            db->pos(1) = m.transpose(ss::float2(worldVertices[SP_VERTEX_X2], worldVertices[SP_VERTEX_Y2]));
            db->texcoord(1) = ss::float2(regionAttachment->uvs[SP_VERTEX_X2], regionAttachment->uvs[SP_VERTEX_Y2]);

            db->pos(2) = db->pos(4) = m.transpose(ss::float2(worldVertices[SP_VERTEX_X3], worldVertices[SP_VERTEX_Y3]));
            db->texcoord(2) = db->texcoord(4) = ss::float2(regionAttachment->uvs[SP_VERTEX_X3], regionAttachment->uvs[SP_VERTEX_Y3]);

            db->pos(5) = m.transpose(ss::float2(worldVertices[SP_VERTEX_X4], worldVertices[SP_VERTEX_Y4]));
            db->texcoord(5) = ss::float2(regionAttachment->uvs[SP_VERTEX_X4], regionAttachment->uvs[SP_VERTEX_Y4]);
        }
        else if (attachment->type == SP_ATTACHMENT_MESH) {
            spMeshAttachment* mesh = (spMeshAttachment*)attachment;

            //TODO: support more than MAX_VERTEX_COUNT(draw them by pieces)

            if (mesh->verticesCount > MAX_VERTEX_COUNT) continue;
            if (mesh->trianglesCount > MAX_VERTEX_COUNT) continue;
            ref_texture = (ss_texture2d_resource_ref*)((spAtlasRegion*)mesh->rendererObject)->page->rendererObject;

            ss_texture2d* texture = ref_texture->get();
            if (texture == nullptr){
                continue;
            }
            spMeshAttachment_computeWorldVertices(mesh, slot, worldVertices);

            ss::color color(
                skeleton->r * slot->r,
                skeleton->g * slot->g,
                skeleton->b * slot->b,
                skeleton->a * slot->a);

            db->prepare(texture, SS_PT_TRIANGLELIST, mesh->trianglesCount);

            for (int i = 0; i < mesh->trianglesCount; ++i) {
                int index = mesh->triangles[i] << 1;
                db->diffuse(i) = color;
                db->pos(i) = m.transpose(ss::float2(worldVertices[index], worldVertices[index + 1]));
                db->texcoord(i) = ss::float2(mesh->uvs[index], mesh->uvs[index + 1]);
            }
        }
        else if (attachment->type == SP_ATTACHMENT_SKINNED_MESH) {
            spSkinnedMeshAttachment* mesh = (spSkinnedMeshAttachment*)attachment;
            if (mesh->uvsCount > MAX_VERTEX_COUNT) continue;
            if (mesh->trianglesCount > MAX_VERTEX_COUNT) continue;
            ref_texture = (ss_texture2d_resource_ref*)((spAtlasRegion*)mesh->rendererObject)->page->rendererObject;

            ss_texture2d* texture = ref_texture->get();

            spSkinnedMeshAttachment_computeWorldVertices(mesh, slot, worldVertices);

            ss::color color(
                skeleton->r * slot->r,
                skeleton->g * slot->g,
                skeleton->b * slot->b,
                skeleton->a * slot->a);

            db->prepare(texture, SS_PT_TRIANGLELIST, mesh->trianglesCount);

            for (int i = 0; i < mesh->trianglesCount; ++i) {
                int index = mesh->triangles[i] << 1;
                db->diffuse(i) = color;
                db->pos(i) = m.transpose(ss::float2(worldVertices[index], worldVertices[index + 1]));
                db->texcoord(i) = ss::float2(mesh->uvs[index], mesh->uvs[index + 1]);
            }
        }
    }
}