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

"    if (sceneParams.traceMode == 0)\n"
"    {\n"
"       if(materialIdent(payloadIn.mNo, EMISSIVE))\n"
"       {\n"
"          payloadIn.mNo = int(matCB[gl_InstanceID].materialNo.x);\n"
"          return;\n"
"       }\n"

"       difTex.xyz = PayloadCalculate_OneRay(payloadIn.RecursionCnt, payloadIn.hitPosition, difTex, speTex,\n"
"                                            normalMap, payloadIn.hitInstanceId);\n"

"       payloadIn.depth = getDepth();\n"
"       payloadIn.normal = normalMap;\n"
"       payloadIn.color = difTex.xyz;\n"
"       payloadIn.hit = true;\n"
"       payloadIn.Alpha = difTex.w;\n"
"    }\n"
"    else\n"
"    {\n"
////////PathTracing
"       payloadIn.normal = normalMap;\n"
"       payloadIn.depth = getDepth();\n"
"       payloadIn.hitInstanceId = gl_InstanceID;\n"

"       if(!materialIdent(payloadIn.mNo, NEE))\n"
"       {\n"
"          payloadIn.color = PayloadCalculate_PathTracing(payloadIn.RecursionCnt, payloadIn.hitPosition,\n"
"                                                         difTex, speTex, normalMap,\n"
"                                                         payloadIn.throughput, payloadIn.hitInstanceId);\n"
"       }\n"
"    }\n"

"    payloadIn.mNo = int(matCB[gl_InstanceID].materialNo.x);\n"
"}\n";
