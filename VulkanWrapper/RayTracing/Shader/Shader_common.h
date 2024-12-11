//*****************************************************************************************//
//**                             Shader_common.h                                         **//
//*****************************************************************************************//

char* Shader_common =

"#version 460\n"
"#extension GL_EXT_ray_tracing : enable\n"
"#extension GL_EXT_nonuniform_qualifier : enable\n"//gl_InstanceIDで必要

"#define NumLightMax 256 \n"
"const float AIR_RefractiveIndex = 1.0f;\n"

"layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;\n"

"layout(binding = 2, set = 0) uniform SceneParameters {\n"
"    mat4 projectionToWorld;\n"
"    vec4 cameraPosition;\n"
"    vec4 emissivePosition[NumLightMax];\n"//.w:onoff
"    vec4 numEmissive;\n"//.xのみ
"    vec4 GlobalAmbientColor;\n"
"    vec4 emissiveNo[NumLightMax];"//.xのみ
"    vec4 TMin_TMax;\n"//.x, .y
"    vec4 maxRecursion;\n"//x:, y:maxNumInstance
"} sceneParams;\n"

"struct MaterialCB {\n"
"    mat4 mvp;\n"
"    vec4 Diffuse;\n"
"    vec4 Speculer; \n"
"    vec4 Ambient;\n"
"    vec4 shininess;\n"//x:
"    vec4 RefractiveIndex;\n"//x:屈折率
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
"    bool reTry;\n"
"    bool hit;\n"
"    float Alpha;\n"
"    int RecursionCnt;\n"
"    int pLightID;\n"//ポイントライトチェック用
"    int mNo;\n"
"    vec4 lightst;\n"//レンジ, 減衰1, 減衰2, 減衰3
"    float depth;\n"
"    int hitInstanceId;\n"
"    uint EmissiveIndex;\n"
"};\n"

"layout(location = 0) rayPayloadEXT vkRayPayload payload;\n";