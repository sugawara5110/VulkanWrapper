//*****************************************************************************************//
//**                             Shader_emissiveHit.h                                    **//
//*****************************************************************************************//

char* Shader_emissiveHit =

"layout(location = 1) rayPayloadInEXT vkRayPayload payloadIn;\n"

"void main()\n"
"{\n"
"    int mNo = int(matCB[gl_InstanceID].materialNo.x);\n"
"    payloadIn.lightst = matCB[gl_InstanceID].lightst;\n"
"    vec4 difTex = getDifPixel();\n"
"    payloadIn.hitPosition = HitWorldPosition();\n"
"    payloadIn.reTry = false;\n"
//ヒットした位置のテクスチャの色をpayloadIn.color格納する
//////点光源
"    bool pay_mNoF = materialIdent(payloadIn.mNo, EMISSIVE);\n"
"    bool mNoF     = materialIdent(mNo, EMISSIVE);\n"
"    if(pay_mNoF && mNoF) {\n"
"       payloadIn.color = difTex.xyz;\n"
"       if(gl_InstanceID != payloadIn.pLightID || difTex.w <= 0.0f) {\n"
"          payloadIn.reTry = true;\n"//目標の点光源以外の場合素通り
"       }\n"
"    }\n"

//////影
"    if( \n"
"       !materialIdent(mNo, EMISSIVE) && \n"
"       materialIdent(payloadIn.mNo, EMISSIVE) || \n"
"       (mNo == METALLIC || mNo == NONREFLECTION) && \n"
"       materialIdent(payloadIn.mNo, METALLIC) \n"
"       ) {\n"
"       if(difTex.w >= 1.0f) {\n"
"          payloadIn.color = vec3(0.0f, 0.0f, 0.0f);\n"
"       }\n"
"       else {\n"
"          payloadIn.reTry = true;\n"
"       }\n"
"    }\n"
"}\n";