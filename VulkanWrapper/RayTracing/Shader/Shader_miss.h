//*****************************************************************************************//
//**                             Shader_miss.h                                           **//
//*****************************************************************************************//

char* Shader_miss =

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

"layout(location = rayPayloadInEXT_location) rayPayloadInEXT vkRayPayload payloadIn;\n"

"void main()\n"
"{\n"
"    payloadIn.color = vec3(0.0, 0.0, 0.0);\n"
"    payloadIn.hit = false;\n"
"    payloadIn.reTry = false;\n"
"}\n";
