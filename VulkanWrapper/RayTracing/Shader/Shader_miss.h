//*****************************************************************************************//
//**                             Shader_miss.h                                           **//
//*****************************************************************************************//

char* Shader_miss =

"layout(location = 1) rayPayloadInEXT vkRayPayload payloadIn;\n"

"void main()\n"
"{\n"
"    payloadIn.color = vec3(0.0, 0.0, 0.0);\n"
"    payloadIn.hit = false;\n"
"    payloadIn.reTry = false;\n"
"}\n";
