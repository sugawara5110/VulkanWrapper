//*****************************************************************************************//
//**                             Shader_closesthit_NormalMapTest.h                       **//
//*****************************************************************************************//

char* Shader_closesthit_NormalMapTest =

"layout(location = 1) rayPayloadInEXT vkRayPayload payloadIn;\n"

"void main()\n"
"{\n"
"    payloadIn.hitPosition = HitWorldPosition();\n"
//テクスチャ取得
"    vec4 difTex = getDifPixel();\n"
"    vec3 normalMap = getNorPixel();\n"
"    vec3 speTex = getSpePixel();\n"

"    vec3 normal = getNormal() * mat3(gl_ObjectToWorld3x4EXT);\n"

"    payloadIn.reTry = false;\n"

"    payloadIn.color = normalMap;\n"
"    payloadIn.hit = true;\n"
"    payloadIn.Alpha = difTex.w;\n"
"    payloadIn.mNo = int(matCB[gl_InstanceID].materialNo.x);\n"
"}\n";
