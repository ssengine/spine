CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(Spine)

ADD_DEFINITIONS(-DLUA_COMPAT_ALL -DLUA_BUILD_AS_DLL)

IF (MSVC)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
ENDIF (MSVC)

OPTION(SS_LUAVERSION_LUAJIT "" ON)

IF (SS_LUAVERSION_LUAJIT)
	SET(SS_LUA_VERSION LUAJIT)
	SET(SS_LUA_LIBNAME lua51)
ELSE()
	ERROR("No lua version")
ENDIF()

SET(LUA_INCLUDE $ENV{${SS_LUA_VERSION}_INCLUDE})
SET(LUA_LIB $ENV{${SS_LUA_VERSION}_LIB})

INCLUDE_DIRECTORIES($ENV{SS_INCLUDE_PATH} ${LUA_INCLUDE})

INCLUDE_DIRECTORIES(include)

SET(LOCAL_SOURCES_FILES
	src/main.cpp
	src/ss_atlas.cpp
	src/ss_animation.cpp
	src/ss_drawable.h
	src/ss_drawable.cpp
	src/ss_extension.cpp

	src/spine/Animation.c
	src/spine/AnimationState.c
	src/spine/AnimationStateData.c
	src/spine/Atlas.c
	src/spine/AtlasAttachmentLoader.c
	src/spine/Attachment.c
	src/spine/AttachmentLoader.c
	src/spine/Bone.c
	src/spine/BoneData.c
	src/spine/BoundingBoxAttachment.c
	src/spine/Event.c
	src/spine/EventData.c
	src/spine/extension.c
	src/spine/IkConstraint.c
	src/spine/IkConstraintData.c
	src/spine/Json.c
	src/spine/Json.h
	src/spine/MeshAttachment.c
	src/spine/RegionAttachment.c
	src/spine/Skeleton.c
	src/spine/SkeletonBounds.c
	src/spine/SkeletonData.c
	src/spine/SkeletonJson.c
	src/spine/Skin.c
	src/spine/SkinnedMeshAttachment.c
	src/spine/Slot.c
	src/spine/SlotData.c
	)

LINK_LIBRARIES(${SS_LUA_LIBNAME} SSCore)
LINK_DIRECTORIES($ENV{SS_LIB_PATH} ${LUA_LIB})

ADD_LIBRARY(spine_native
	SHARED
	${LOCAL_SOURCES_FILES}
)

