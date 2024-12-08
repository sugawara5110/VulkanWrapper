//*****************************************************************************************//
//**                             Shader_traceRay_OneRay.h                                **//
//*****************************************************************************************//

char* Shader_traceRay_OneRay =

///////////////////////光源へ光線を飛ばす, ヒットした場合明るさが加算//////////////////////////
"vec3 EmissivePayloadCalculate(in int RecursionCnt, in vec3 hitPosition, \n"
"                                in vec3 difTexColor, in vec3 speTexColor, in vec3 normalMap, in vec3 normal)\n"
"{\n"
"    MaterialCB mcb = matCB[gl_InstanceID];\n"
"    int mNo = int(mcb.materialNo.x);\n"
"    vec3 ret = difTexColor;\n"

"    bool mf = materialIdent(mNo, EMISSIVE);\n"
"    if(!mf) {\n"//emissive以外

"       LightOut emissiveColor = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"
"       LightOut Out;\n"
"       payload.hitPosition = hitPosition;\n"
"       float tmin = sceneParams.TMin_TMax.x;\n"
"       float tmax = sceneParams.TMin_TMax.y;\n"
"       RecursionCnt++;\n"

"       vec3 SpeculerCol = mcb.Speculer.xyz;\n"
"       vec3 Diffuse = mcb.Diffuse.xyz;\n"
"       vec3 Ambient = mcb.Ambient.xyz + sceneParams.GlobalAmbientColor.xyz;\n"
"       float shininess = mcb.shininess.x;\n"

"       if(RecursionCnt <= sceneParams.maxRecursion.x) {\n"

"          const uint RayFlags = gl_RayFlagsCullBackFacingTrianglesEXT;\n"
//光源計算
"          for(int i = 0; i < sceneParams.numEmissive.x; i++) {\n"

"              vec4 emissivePosition = sceneParams.emissivePosition[i];\n"

"              if(emissivePosition.w == 1.0f) {\n"

"                 vec3 lightVec = normalize(emissivePosition.xyz - hitPosition);\n"
"                 vec3 direction = lightVec;\n"
"                 payload.pLightID = int(sceneParams.emissiveNo[i].x);\n"
"                 bool loop = true;\n"
"                 payload.hitPosition = hitPosition;\n"
"                 while(loop){\n"
"                    payload.mNo = EMISSIVE;\n"//処理分岐用
"                    vec3 origin = payload.hitPosition;\n"

"                    traceRayEXT(\n"
"                        topLevelAS,\n"
"                        RayFlags,\n"
"                        0xff,\n"
"                        1,\n"//sbtRecordOffset
"                        0,\n"//sbtRecordStride
"                        1,\n"//missIndex
"                        origin,\n"
"                        tmin,\n"
"                        direction,\n"
"                        tmax,\n"
"                        0\n"//layoutのlocation番号
"                    );\n"

"                    loop = payload.reTry;\n"
"                 }\n"

"                 Out = PointLightCom(SpeculerCol, Diffuse, Ambient, normalMap, emissivePosition, \n"
"                                     hitPosition, payload.lightst, payload.color, sceneParams.cameraPosition.xyz, shininess);\n"

"                 emissiveColor.Diffuse += Out.Diffuse;\n"
"                 emissiveColor.Speculer += Out.Speculer;\n"
"              }\n"
"          }\n"
"       }\n"
//最後にテクスチャの色に掛け合わせ
"       difTexColor *= emissiveColor.Diffuse;\n"
"       speTexColor *= emissiveColor.Speculer;\n"
"       ret = difTexColor + speTexColor;\n"
"    }\n"
"    return ret;\n"
"}\n"

///////////////////////反射方向へ光線を飛ばす, ヒットした場合ピクセル値乗算///////////////////////
"vec3 MetallicPayloadCalculate(in int RecursionCnt, in vec3 hitPosition, \n"
"                                in vec3 difTexColor, in vec3 normal, inout int hitInstanceId)\n"
"{\n"
"    int mNo = int(matCB[gl_InstanceID].materialNo.x);\n"
"    vec3 ret = difTexColor;\n"

"    hitInstanceId = gl_InstanceID; \n"//自身のID書き込み

"    if(materialIdent(mNo, METALLIC)) {\n"//METALLIC

"       RecursionCnt++;\n"
"       payload.RecursionCnt = RecursionCnt;\n"
//視線ベクトル 
"       vec3 eyeVec = gl_WorldRayDirectionEXT;\n"
//反射ベクトル
"       vec3 reflectVec = reflect(eyeVec, normalize(normal));\n"
"       vec3 direction = reflectVec;\n"//反射方向にRayを飛ばす

"       float tmin = sceneParams.TMin_TMax.x;\n"
"       float tmax = sceneParams.TMin_TMax.y;\n"

"       if (RecursionCnt <= sceneParams.maxRecursion.x) {\n"
"           payload.hitPosition = hitPosition;\n"
"           vec3 origin = payload.hitPosition;\n"

"           traceRayEXT(\n"
"               topLevelAS,\n"
"               gl_RayFlagsCullBackFacingTrianglesEXT,\n"
"               0xff,\n"
"               0,\n"//sbtRecordOffset
"               0,\n"//sbtRecordStride
"               0,\n"//missIndex
"               origin,\n"
"               tmin,\n"
"               direction,\n"
"               tmax,\n"
"               0\n"//ペイロードインデックス
"           );\n"

"       }\n"

"       vec3 outCol = vec3(0.0f, 0.0f, 0.0f);\n"
"       if (payload.hit) {\n"
"           outCol = difTexColor * payload.color;\n"//ヒットした場合映り込みとして乗算
"           hitInstanceId = payload.hitInstanceId;\n"//ヒットしたID書き込み
"           int hitmNo = payload.mNo;\n"
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
"vec3 Translucent(in int RecursionCnt, in vec3 hitPosition, in vec4 difTexColor, in vec3 normal)\n"
"{\n"
"    MaterialCB mcb = matCB[gl_InstanceID];\n"
"    int mNo = int(mcb.materialNo.x);\n"
"    vec3 ret = difTexColor.xyz;\n"

"    if(materialIdent(mNo, TRANSLUCENCE)) {\n"

"       float Alpha = difTexColor.w;\n"
"       RecursionCnt++;\n"
"       payload.RecursionCnt = RecursionCnt;\n"
"       float tmin = sceneParams.TMin_TMax.x;\n"
"       float tmax = sceneParams.TMin_TMax.y;\n"
//視線ベクトル 
"       vec3 eyeVec = gl_WorldRayDirectionEXT;\n"
"       vec3 direction = normalize(eyeVec + -normal * mcb.RefractiveIndex.x);\n"

"       if (RecursionCnt <= sceneParams.maxRecursion.x) {\n"
"           payload.hitPosition = hitPosition;\n"
"           vec3 origin = payload.hitPosition;\n"

"           traceRayEXT(\n"
"               topLevelAS,\n"
"               gl_RayFlagsCullBackFacingTrianglesEXT,\n"
"               0xff,\n"
"               0,\n"//sbtRecordOffset
"               0,\n"//sbtRecordStride
"               0,\n"//missIndex
"               origin,\n"
"               tmin,\n"
"               direction,\n"
"               tmax,\n"
"               0\n"//ペイロードインデックス
"           );\n"
"       }\n"
//アルファ値の比率で元の色と光線衝突先の色を配合
"       ret = payload.color * (1.0f - Alpha) + difTexColor.xyz * Alpha;\n"
"    }\n"
"    return ret;\n"
"}\n";