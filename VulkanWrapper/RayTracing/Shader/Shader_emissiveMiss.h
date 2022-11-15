//*****************************************************************************************//
//**                             Shader_emissiveMiss.h                                   **//
//*****************************************************************************************//

char* Shader_emissiveMiss =

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
"    vec4 lightst;\n"//ÉåÉìÉW, å∏êä1, å∏êä2, å∏êä3
"    float depth;\n"
"};\n"

"layout(location = rayPayloadInEXT_location) rayPayloadInEXT vkRayPayload payloadIn;\n"

"void main()\n"
"{\n"
"    payloadIn.color = vec3(0.5, 0.5, 0.5);\n"
"    payloadIn.reTry = false;\n"
"}\n";