//*****************************************************************************************//
//**                             Shader_emissiveMiss.h                                   **//
//*****************************************************************************************//

char* Shader_emissiveMiss =

"layout(location = 1) rayPayloadInEXT vkRayPayload payloadIn;\n"

"void main()\n"
"{\n"
"    payloadIn.color = vec3(0.0, 0.0, 0.0);\n"
"    payloadIn.hit = false;\n"
"    payloadIn.reTry = false;\n"
"    payloadIn.mNo = NONREFLECTION;\n"
"}\n";