#pragma once

#include <spine/Skeleton.h>
#include <spine/AnimationState.h>

typedef struct ss_core_context ss_core_context;
typedef struct ss_resource_ref ss_resource_ref;
typedef struct render2d_context render2d_context;

struct ss_spine_drawable
{
public:
    spSkeleton* skeleton;
    spAnimationState* state;
    float timeScale;

    ss_spine_drawable(spSkeletonData* skeleton, ss_core_context* C, ss_resource_ref* res);
    ~ss_spine_drawable();

    void update(float deltaTime);

    void draw(render2d_context* RC) const;
private:
    ss_resource_ref* res_;
    ss_core_context* C_;
};
