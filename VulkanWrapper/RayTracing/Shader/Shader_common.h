﻿//*****************************************************************************************//
//**                             Shader_common.h                                         **//
//*****************************************************************************************//

char* Shader_common =

"#version 460\n"
"#extension GL_EXT_ray_tracing : enable\n"
"#extension GL_EXT_nonuniform_qualifier : enable\n"//gl_InstanceIDで必要

"#define NumLightMax 256 \n"

"const float PI = 3.1415926;\n"
"vec3 local_normal = vec3(0.0f, 0.0f, 1.0f);\n"
"const float AIR_RefractiveIndex = 1.0f;\n"

"#define NONREFLECTION  64 \n" //0b1000000
"#define METALLIC       32 \n" //0b0100000
"#define EMISSIVE       16 \n" //0b0010000
"#define TRANSLUCENCE   8  \n" //0b0001000
"#define NEE            4  \n" //0b0000100
"#define NEE_PATHTRACER 2  \n" //0b0000010
"#define NONE           1  \n" //0b0000001

"layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;\n"

"layout(binding = 2, set = 0) uniform SceneParameters {\n"
"    mat4 prevViewProjection;\n"
"    mat4 projectionToWorld;\n"
"    mat4 ImageBasedLighting_Matrix;\n"
"    vec4 cameraPosition;\n"
"    vec4 emissivePosition[NumLightMax];\n"//.w:onoff
"    vec4 numEmissive;\n"//.xのみ
"    vec4 GlobalAmbientColor;\n"
"    vec4 emissiveNo[NumLightMax];"//x:emissiveNo, y:OutlineSize
"    vec4 TMin_TMax;\n"//.x, .y
"    vec4 frameReset_DepthRange_NorRange;\n"//.x:フレームインデックスリセット(1.0でリセット), .y:深度レンジ, .z:法線レンジ
"    vec4 maxRecursion;\n"//x:, y:maxNumInstance
"    uint traceMode;\n"
"    uint SeedFrame;\n"
"    float IBL_size;\n"
"    bool useImageBasedLighting;\n"
"} sceneParams;\n"

"struct MaterialCB {\n"
"    mat4 mvp;\n"
"    vec4 Diffuse;\n"
"    vec4 Speculer; \n"
"    vec4 Ambient;\n"
"    vec4 shininess;\n"//x:
"    vec4 RefractiveIndex_roughness;\n"//x:屈折率, y:粗さ
"    vec4 materialNo;\n"//x:
"    vec4 addColor;\n"
"    vec4 lightst;\n"//レンジ, 減衰1, 減衰2, 減衰3
"};\n"

"layout(binding = 0, set = 4) uniform Materials {\n"
"    MaterialCB matCB[replace_NUM_MAT_CB];\n"//replace_NUM_MAT_CB:書き換え
"};\n"

"struct vkRayPayload\n"
"{\n"
"    vec3 color;\n"
"    vec3 hitPosition;\n"
"    vec3 normal;\n"
"    vec3 throughput;\n"
"    bool reTry;\n"
"    bool hit;\n"
"    float Alpha;\n"
"    uint RecursionCnt;\n"
"    uint mNo;\n"
"    vec4 lightst;\n"//レンジ, 減衰1, 減衰2, 減衰3
"    float depth;\n"
"    int hitInstanceId;\n"
"    int EmissiveIndex;\n"
"    uint Seed;\n"
"};\n";