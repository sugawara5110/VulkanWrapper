//*****************************************************************************************//
//**                             Shader_closesthit.h                                     **//
//*****************************************************************************************//

char* Shader_closesthit =

"layout(location = 1) rayPayloadInEXT vkRayPayload payloadIn;\n"

"void main()\n"
"{\n"
"    payloadIn.hitPosition = HitWorldPosition();\n"
//テクスチャ取得
"    vec4 difTex = getDifPixel();\n"
"    vec3 normalMap = getNorPixel();\n"
"    vec3 speTex = getSpePixel();\n"

//深度取得
"    if(payloadIn.depth == -1.0f)\n" 
"    {\n"
"       payloadIn.depth = getDepth();\n"
"    }\n"

"    if(materialIdent(payloadIn.mNo, EMISSIVE))\n"
"    {\n"
"       payloadIn.mNo = int(matCB[gl_InstanceID].materialNo.x);\n"
"       return;\n"
"    }\n"

"    difTex.xyz = PayloadCalculate_OneRay(payloadIn.RecursionCnt, payloadIn.hitPosition, difTex, speTex,\n"
"                                         normalMap, payloadIn.hitInstanceId);\n"

"    payloadIn.color = difTex.xyz;\n"
"    payloadIn.hit = true;\n"
"    payloadIn.Alpha = difTex.w;\n"
"    payloadIn.mNo = int(matCB[gl_InstanceID].materialNo.x);\n"
"}\n";
