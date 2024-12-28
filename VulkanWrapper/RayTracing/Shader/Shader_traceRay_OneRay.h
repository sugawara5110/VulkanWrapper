//*****************************************************************************************//
//**                             Shader_traceRay_OneRay.h                                **//
//*****************************************************************************************//

char* Shader_traceRay_OneRay =

///////////////////////光源へ光線を飛ばす, ヒットした場合明るさが加算//////////////////////////
"vec3 EmissivePayloadCalculate(in uint RecursionCnt, in vec3 hitPosition, \n"
"                              in vec3 difTexColor, in vec3 speTexColor, in vec3 normal)\n"
"{\n"
"    MaterialCB mcb = matCB[gl_InstanceID];\n"
"    uint mNo = int(mcb.materialNo.x);\n"
"    vec3 ret = difTexColor;\n"

"    LightOut emissiveColor = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"
"    LightOut Out;\n"

"    vec3 SpeculerCol = mcb.Speculer.xyz;\n"
"    vec3 Diffuse = mcb.Diffuse.xyz;\n"
"    vec3 Ambient = mcb.Ambient.xyz + sceneParams.GlobalAmbientColor.xyz;\n"
"    float shininess = mcb.shininess.x;\n"

"    for(int i = 0; i < sceneParams.numEmissive.x; i++) {\n"

"        vec4 emissivePosition = sceneParams.emissivePosition[i];\n"

"        if(emissivePosition.w == 1.0f) {\n"

"           vec3 lightVec = normalize(emissivePosition.xyz - hitPosition);\n"
"           vec3 direction = lightVec;\n"
"           payload.hitPosition = hitPosition;\n"
"           payload.mNo = EMISSIVE; \n"//処理分岐用

"           traceRay(RecursionCnt,\n"
"                    gl_RayFlagsCullBackFacingTrianglesEXT,\n"
"                    0,\n"
"                    1,\n"
"                    direction);\n"

"           if(materialIdent(payload.mNo, EMISSIVE))\n"//狙い通り光源に当たった場合のみ色計算
"           {\n"
"               emissivePosition.xyz = payload.hitPosition;\n"
"               Out = PointLightCom(SpeculerCol, Diffuse, Ambient, normal, emissivePosition, \n"
"                                   hitPosition, payload.lightst, payload.color, sceneParams.cameraPosition.xyz, shininess);\n"

"               emissiveColor.Diffuse += Out.Diffuse;\n"
"               emissiveColor.Speculer += Out.Speculer;\n"
"           }\n"
"        }\n"
"    }\n"
//最後にテクスチャの色に掛け合わせ
"    difTexColor *= emissiveColor.Diffuse;\n"
"    speTexColor *= emissiveColor.Speculer;\n"
"    return difTexColor + speTexColor;\n"
"}\n"

///////////////////////反射方向へ光線を飛ばす, ヒットした場合ピクセル値乗算///////////////////////
"vec3 MetallicPayloadCalculate(in uint RecursionCnt, in vec3 hitPosition, \n"
"                              in vec3 difTexColor, in vec3 normal, inout int hitInstanceId)\n"
"{\n"
"    uint mNo = int(matCB[gl_InstanceID].materialNo.x);\n"
"    vec3 ret = difTexColor;\n"

"    hitInstanceId = gl_InstanceID; \n"//自身のID書き込み

"    if(materialIdent(mNo, METALLIC)) {\n"//METALLIC

"       vec3 eyeVec = gl_WorldRayDirectionEXT;\n"
//反射ベクトル
"       vec3 reflectVec = reflect(eyeVec, normalize(normal));\n"
"       vec3 direction = reflectVec;\n"//反射方向にRayを飛ばす

"       payload.hitPosition = hitPosition; \n"
"       payload.mNo = METALLIC; \n"//処理分岐用

"       traceRay(RecursionCnt,\n"
"                gl_RayFlagsCullBackFacingTrianglesEXT,\n"
"                0,\n"
"                0,\n"
"                direction);\n"

"       vec3 outCol = vec3(0.0f, 0.0f, 0.0f);\n"
"       if (payload.hit) {\n"
"           outCol = difTexColor * payload.color;\n"//ヒットした場合映り込みとして乗算
"           hitInstanceId = payload.hitInstanceId;\n"//ヒットしたID書き込み
"           uint hitmNo = payload.mNo;\n"
"           if(materialIdent(hitmNo, EMISSIVE)){\n"
"              outCol = payload.color;\n"
"           }\n"
"       }\n"
"       else {\n"
"           outCol = difTexColor;\n"//ヒットしなかった場合映り込み無しで元のピクセル書き込み
"       }\n"
"       ret = outCol;\n"
"    }\n"
"    return ret;\n"
"}\n"

////////////////////////////////////////半透明//////////////////////////////////////////
"vec3 Translucent(in uint RecursionCnt, in vec3 hitPosition, in vec4 difTexColor, in vec3 normal)\n"
"{\n"
"    MaterialCB mcb = matCB[gl_InstanceID];\n"
"    uint mNo = int(mcb.materialNo.x);\n"
"    vec3 ret = difTexColor.xyz;\n"

"    if(materialIdent(mNo, TRANSLUCENCE)) {\n"

"       float Alpha = difTexColor.w;\n"

"       float in_eta = AIR_RefractiveIndex;\n"
"       float out_eta = mcb.RefractiveIndex_roughness.x;\n"

"       vec3 r_eyeVec = -gl_WorldRayDirectionEXT;\n"
"       float norDir = dot(r_eyeVec, normal);\n"
"       if (norDir < 0.0f)\n"
"       {\n"
"           normal *= -1.0f;\n"
"           in_eta = mcb.RefractiveIndex_roughness.x;\n"
"           out_eta = AIR_RefractiveIndex;\n"
"       }\n"

"       vec3 eyeVec = gl_WorldRayDirectionEXT;\n"
"       float eta = in_eta / out_eta;\n"
"       vec3 direction = refract(eyeVec, normalize(normal), eta);\n"

"       payload.hitPosition = hitPosition;\n"
"       payload.mNo = TRANSLUCENCE; \n"//処理分岐用

"       traceRay(RecursionCnt,\n"
"                gl_RayFlagsCullBackFacingTrianglesEXT,\n"
"                0,\n"
"                0,\n"
"                direction);\n"

//アルファ値の比率で元の色と光線衝突先の色を配合
"       ret = payload.color * (1.0f - Alpha) + difTexColor.xyz * Alpha;\n"
"    }\n"
"    return ret;\n"
"}\n"

////////////////////////////////////////ONE_RAY//////////////////////////////////////////
"vec3 PayloadCalculate_OneRay(in uint RecursionCnt, in vec3 hitPosition,\n"
"                             in vec4 difTex, in vec3 speTex, in vec3 normalMap,\n"
"                             inout int hitInstanceId)\n"
"{\n"
//光源への光線
"    difTex.xyz = EmissivePayloadCalculate(RecursionCnt, hitPosition, difTex.xyz, speTex, normalMap);\n"
//反射方向への光線
"    difTex.xyz = MetallicPayloadCalculate(RecursionCnt, hitPosition, difTex.xyz, normalMap, hitInstanceId);\n"
//半透明
"    difTex.xyz = Translucent(RecursionCnt, hitPosition, difTex, normalMap);\n"

"    return difTex.xyz;\n"
"}\n";