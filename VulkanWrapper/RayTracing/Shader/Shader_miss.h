//*****************************************************************************************//
//**                                                                                     **//
//**                             Shader_miss.h                                           **//
//**                                                                                     **//
//*****************************************************************************************//

char* Shader_miss =

"#version 460\n"
"#extension GL_EXT_ray_tracing : enable\n"

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
"};\n"

"layout(location = 0) rayPayloadInEXT vkRayPayload payload;\n"

"void main()\n"
"{\n"
"    payload.color = vec3(0.0, 0.1, 0.2);\n"
"}\n";
