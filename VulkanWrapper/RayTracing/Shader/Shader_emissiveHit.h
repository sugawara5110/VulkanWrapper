//*****************************************************************************************//
//**                             Shader_emissiveHit.h                                    **//
//*****************************************************************************************//

char* Shader_emissiveHit =

"layout(location = 1) rayPayloadInEXT vkRayPayload payloadIn;\n"

"int getEmissiveIndex()\n"
"{\n"
"    int ret = -1;\n"
"    for (int i = 0; i < 256; i++)\n"
"    {\n"
"        if (sceneParams.emissiveNo[i].x == gl_InstanceID)\n"
"        {\n"
"            ret = i;\n"
"            break;\n"
"        }\n"
"	 }\n"
"	 return ret;\n"
"}\n"

"void main()\n"
"{\n"
"    payloadIn.lightst = matCB[gl_InstanceID].lightst;\n"
"    vec4 difTex = getDifPixel();\n"
"    payloadIn.normal = getNorPixel();\n"
"    payloadIn.hitPosition = HitWorldPosition();\n"

"    if (difTex.w < 1.0f)\n"
"    {\n"
"        payloadIn.reTry = true;\n"
"        return;\n"
"    }\n"

"    payloadIn.hitInstanceId = gl_InstanceID;\n"
"    payloadIn.EmissiveIndex = getEmissiveIndex();\n"
"    payloadIn.hit = true;\n"
"    payloadIn.color = difTex.xyz;\n"
"    payloadIn.mNo = int(matCB[gl_InstanceID].materialNo.x);\n"
"}\n";