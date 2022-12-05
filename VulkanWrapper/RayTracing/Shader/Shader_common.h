//*****************************************************************************************//
//**                             Shader_common.h                                         **//
//*****************************************************************************************//

char* Shader_common =

"#version 460\n"
"#extension GL_EXT_ray_tracing : enable\n"
"#extension GL_EXT_nonuniform_qualifier : enable\n"//gl_InstanceIDで必要

"#define NumLightMax 256 \n"

"layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;\n"

"layout(binding = 2, set = 0) uniform SceneParameters {\n"
"    mat4 projectionToWorld;\n"
"    vec4 cameraPosition;\n"
"    vec4 emissivePosition[NumLightMax];\n"//.w:onoff
"    vec4 numEmissive;\n"//.xのみ
"    vec4 GlobalAmbientColor;\n"
"    vec4 emissiveNo[NumLightMax];"//.xのみ
"    vec4 dDirection;\n"
"    vec4 dLightColor;\n"
"    vec4 dLightst;\n"//.x:オンオフ
"    vec4 TMin_TMax;\n"//.x, .y
"    vec4 maxRecursion;\n"//x:
"} sceneParams;\n"

"struct MaterialCB {\n"
"    vec4 Diffuse;\n"
"    vec4 Speculer; \n"
"    vec4 Ambient;\n"
"    vec4 lightst;\n"//レンジ, 減衰1, 減衰2, 減衰3
"    vec4 shininess;\n"//x:
"    vec4 RefractiveIndex;\n"//x:屈折率
"    vec4 AlphaBlend;\n"//x:
"    vec4 materialNo;\n"//x:
"    mat4 world;\n"//使用してない, 後で消す
"};\n"

"layout(binding = 0, set = 4) uniform Materials {\n"
"    MaterialCB matCB[256];\n"//後で書き変えれるようにする
"};\n"

"struct vkRayPayload\n"
"{\n"
"    vec3 color;\n"
"    vec3 hitPosition;\n"
"    bool reTry;\n"
"    bool hit;\n"
"    float Alpha;\n"
"    int RecursionCnt;\n"
"    int instanceID;\n"
"    int mNo;\n"
"    vec4 lightst;\n"//レンジ, 減衰1, 減衰2, 減衰3
"    float depth;\n"
"};\n";