//*****************************************************************************************//
//**                             Shader_closesthit.h                                     **//
//*****************************************************************************************//

char* Shader_closesthit =

"layout(location = rayPayloadInEXT_location) rayPayloadInEXT vkRayPayload payloadIn;\n"

"void main()\n"
"{\n"
"    payloadIn.hitPosition = HitWorldPosition();\n"
//テクスチャ取得
"    vec4 difTex = getDifPixel();\n"
"    vec3 normalMap = getNorPixel();\n"
"    vec3 speTex = getSpePixel();\n"

"    vec3 normal = getNormal() * mat3(gl_ObjectToWorld3x4EXT);\n"

"    payloadIn.reTry = false;\n"

//深度取得
"    if(payloadIn.depth == -1.0f) {\n"
"       payloadIn.depth = getDepth();\n"
"    }\n"
//光源への光線
"    difTex.xyz = EmissivePayloadCalculate(payloadIn.RecursionCnt, payloadIn.hitPosition, difTex.xyz, speTex, normalMap, normal);\n"
//反射方向への光線
"    difTex.xyz = MetallicPayloadCalculate(payloadIn.RecursionCnt, payloadIn.hitPosition, difTex.xyz, normalMap, payloadIn.hitInstanceId);\n"
//半透明
"    difTex.xyz = Translucent(payloadIn.RecursionCnt, payloadIn.hitPosition, difTex, normalMap); \n"
//アルファブレンド
"    difTex.xyz = AlphaBlend(payloadIn.RecursionCnt, payloadIn.hitPosition, difTex);\n"

"    payloadIn.color = difTex.xyz;\n"
"    payloadIn.hit = true;\n"
"    payloadIn.Alpha = difTex.w;\n"
"}\n";
