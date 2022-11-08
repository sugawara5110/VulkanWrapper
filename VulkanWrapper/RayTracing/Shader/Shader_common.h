//*****************************************************************************************//
//**                                                                                     **//
//**                             Shader_common.h                                         **//
//**                                                                                     **//
//*****************************************************************************************//

char* Shader_common =

"#version 460\n"
"#extension GL_EXT_ray_tracing : enable\n"
"#extension GL_EXT_nonuniform_qualifier : enable\n"//gl_InstanceIDで必要

"#define NumLightMax 256 \n"

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
"    int maxRecursion;\n"
"} sceneParams;\n"

"struct MaterialCB {\n"
"    vec4 Diffuse;\n"
"    vec4 Speculer; \n"
"    vec4 Ambient;\n"
"    float shininess;\n"
"    float RefractiveIndex;\n"//屈折率
"    float AlphaBlend;\n"
"    int materialNo;\n"
"    vec4 lightst;\n"//レンジ, 減衰1, 減衰2, 減衰3
"};\n"

"layout(binding = 8, set = 0) uniform Materials {\n"
"    MaterialCB mat[];\n"
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