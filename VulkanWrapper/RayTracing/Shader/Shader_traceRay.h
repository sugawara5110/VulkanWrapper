//*****************************************************************************************//
//**                             Shader_traceRay.h                                       **//
//*****************************************************************************************//

char* Shader_traceRay =

"layout(location = rayPayloadEXT_location) rayPayloadEXT vkRayPayload payload;\n"

///////////////////////�����֌������΂�, �q�b�g�����ꍇ���邳�����Z//////////////////////////
"vec3 EmissivePayloadCalculate(in int RecursionCnt, in vec3 hitPosition, \n"
"                                in vec3 difTexColor, in vec3 speTexColor, in vec3 normalMap, in vec3 normal)\n"
"{\n"
"    MaterialCB mcb = matCB[gl_InstanceID];\n"
"    int mNo = int(mcb.materialNo.x);\n"
"    vec3 ret = difTexColor;\n"

"    bool mf = materialIdent(mNo, EMISSIVE);\n"
"    if(!mf) {\n"//emissive�ȊO

"       LightOut emissiveColor = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"
"       LightOut Out;\n"
"       payload.hitPosition = hitPosition;\n"
"       float tmin = sceneParams.TMin_TMax.x;\n"
"       float tmax = sceneParams.TMin_TMax.y;\n"
"       RecursionCnt++;\n"

"       vec3 SpeculerCol = mcb.Speculer.xyz;\n"
"       vec3 Diffuse = mcb.Diffuse.xyz;\n"
"       vec3 Ambient = mcb.Ambient.xyz;\n"
"       float shininess = mcb.shininess.x;\n"

"       if(RecursionCnt <= sceneParams.maxRecursion.x) {\n"

"          const uint RayFlags = gl_RayFlagsCullBackFacingTrianglesEXT;\n"
//�_�����v�Z
"          for(int i = 0; i < sceneParams.numEmissive.x; i++) {\n"

"              vec4 emissivePosition = sceneParams.emissivePosition[i];\n"

"              if(emissivePosition.w == 1.0f) {\n"

"                 vec3 lightVec = normalize(emissivePosition.xyz - hitPosition);\n"
"                 vec3 direction = lightVec;\n"
"                 payload.instanceID = int(sceneParams.emissiveNo[i].x);\n"
"                 bool loop = true;\n"
"                 payload.hitPosition = hitPosition;\n"
"                 while(loop){\n"
"                    payload.mNo = EMISSIVE;\n"//��������p
"                    vec3 origin = payload.hitPosition;\n"

"                    if(dot(normal, direction) <= 0){\n"//�@���ɑ΂��Č����Ɍ��������C�̕�����90���ȏ�Ȃ烌�C���΂��Ȃ�
"                        payload.reTry = false;\n"
"                        payload.color = sceneParams.GlobalAmbientColor.xyz;\n"
"                        break;\n"
"                    }\n"

"                    traceRayEXT(\n"
"                        topLevelAS,\n"
"                        RayFlags,\n"
"                        0xff,\n"
"                        em_Index,\n"//sbtRecordOffset
"                        0,\n"//sbtRecordStride
"                        em_Index,\n"//missIndex
"                        origin,\n"
"                        tmin,\n"
"                        direction,\n"
"                        tmax,\n"
"                        rayPayloadEXT_location\n"//�y�C���[�h�C���f�b�N�X
"                    );\n"

"                    loop = payload.reTry;\n"
"                 }\n"

"                 Out = PointLightCom(SpeculerCol, Diffuse, Ambient, normalMap, emissivePosition, \n"
"                                     hitPosition, payload.lightst, payload.color, sceneParams.cameraPosition.xyz, shininess);\n"

"                 emissiveColor.Diffuse += Out.Diffuse;\n"
"                 emissiveColor.Speculer += Out.Speculer;\n"
"              }\n"
"          }\n"
//���s�����v�Z
"          if(sceneParams.dLightst.x == 1.0f){\n"
"             payload.hitPosition = hitPosition;\n"
"             vec3 direction = -sceneParams.dDirection.xyz;\n"
"             bool loop = true;\n"
"             while(loop){\n"
"                payload.mNo = DIRECTIONLIGHT | METALLIC;\n"//��������p
"                vec3 origin = payload.hitPosition;\n"

"                traceRayEXT(\n"
"                    topLevelAS,\n"
"                    RayFlags,\n"
"                    0xff,\n"
"                    em_Index,\n"//sbtRecordOffset
"                    0,\n"//sbtRecordStride
"                    em_Index,\n"//missIndex
"                    origin,\n"
"                    tmin,\n"
"                    direction,\n"
"                    tmax,\n"
"                    rayPayloadEXT_location\n"//�y�C���[�h�C���f�b�N�X
"                );\n"

"                loop = payload.reTry;\n"
"             }\n"

"             Out = DirectionalLightCom(SpeculerCol, Diffuse, Ambient, normalMap, sceneParams.dLightst, sceneParams.dDirection.xyz, \n"
"                                       payload.color, hitPosition, sceneParams.cameraPosition.xyz, shininess);\n"

"             emissiveColor.Diffuse += Out.Diffuse;\n"
"             emissiveColor.Speculer += Out.Speculer;\n"
"          }\n"
"       }\n"
//�Ō�Ƀe�N�X�`���̐F�Ɋ|�����킹
"       difTexColor *= emissiveColor.Diffuse;\n"
"       speTexColor *= emissiveColor.Speculer;\n"
"       ret = difTexColor + speTexColor;\n"
"    }\n"
"    return ret;\n"
"}\n"

///////////////////////���˕����֌������΂�, �q�b�g�����ꍇ�s�N�Z���l��Z///////////////////////
"vec3 MetallicPayloadCalculate(in int RecursionCnt, in vec3 hitPosition, \n"
"                                in vec3 difTexColor, in vec3 normal)\n"
"{\n"
"    int mNo = int(matCB[gl_InstanceID].materialNo.x);\n"
"    vec3 ret = difTexColor;\n"

"    if(materialIdent(mNo, METALLIC)) {\n"//METALLIC

"       RecursionCnt++;\n"
"       payload.RecursionCnt = RecursionCnt;\n"
//�����x�N�g�� 
"       vec3 eyeVec = gl_WorldRayDirectionEXT;\n"
//���˃x�N�g��
"       vec3 reflectVec = reflect(eyeVec, normalize(normal));\n"
"       vec3 direction = reflectVec;\n"//���˕�����Ray���΂�

"       float tmin = sceneParams.TMin_TMax.x;\n"
"       float tmax = sceneParams.TMin_TMax.y;\n"

"       if (RecursionCnt <= sceneParams.maxRecursion.x) {\n"
"           payload.hitPosition = hitPosition;\n"
"           vec3 origin = payload.hitPosition;\n"

"           traceRayEXT(\n"
"               topLevelAS,\n"
"               gl_RayFlagsCullBackFacingTrianglesEXT,\n"
"               0xff,\n"
"               clo_Index,\n"//sbtRecordOffset
"               0,\n"//sbtRecordStride
"               clo_Index,\n"//missIndex
"               origin,\n"
"               tmin,\n"
"               direction,\n"
"               tmax,\n"
"               rayPayloadEXT_location\n"//�y�C���[�h�C���f�b�N�X
"           );\n"

"       }\n"

"       vec3 outCol = vec3(0.0f, 0.0f, 0.0f);\n"
"       if (payload.hit) {\n"
"           outCol = difTexColor * payload.color;\n"//�q�b�g�����ꍇ�f�荞�݂Ƃ��ď�Z
"       }\n"
"       else {\n"
"           outCol = difTexColor;\n"//�q�b�g���Ȃ������ꍇ�f�荞�ݖ����Ō��̃s�N�Z����������
"       }\n"
"       ret = outCol;\n"
"    }\n"

"    return ret;\n"
"}\n"

////////////////////////////////////////������//////////////////////////////////////////
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
//�����x�N�g�� 
"       vec3 eyeVec = gl_WorldRayDirectionEXT;\n"
"       vec3 direction = normalize(eyeVec + -normal * mcb.RefractiveIndex.x);\n"

"       if (RecursionCnt <= sceneParams.maxRecursion.x) {\n"
"           payload.hitPosition = hitPosition;\n"
"           vec3 origin = payload.hitPosition;\n"

"           traceRayEXT(\n"
"               topLevelAS,\n"
"               gl_RayFlagsCullBackFacingTrianglesEXT,\n"
"               0xff,\n"
"               clo_Index,\n"//sbtRecordOffset
"               0,\n"//sbtRecordStride
"               clo_Index,\n"//missIndex
"               origin,\n"
"               tmin,\n"
"               direction,\n"
"               tmax,\n"
"               rayPayloadEXT_location\n"//�y�C���[�h�C���f�b�N�X
"           );\n"
"       }\n"
//�A���t�@�l�̔䗦�Ō��̐F�ƌ����Փː�̐F��z��
"       ret = payload.color * (1.0f - Alpha) + difTexColor.xyz * Alpha;\n"
"    }\n"
"    return ret;\n"
"}\n"

////////////////////////////////////�A���t�@�u�����h//////////////////////////////////////
"vec3 AlphaBlend(in int RecursionCnt, in vec3 hitPosition, in vec4 difTexColor)\n"
"{\n"
"    MaterialCB mcb = matCB[gl_InstanceID];\n"
"    int mNo = int(mcb.materialNo.x);\n"
"    vec3 ret = difTexColor.xyz;\n"
"    float blend = mcb.AlphaBlend.x;\n"
"    float Alpha = difTexColor.w;\n"

"    bool mf = materialIdent(mNo, TRANSLUCENCE);\n"
"    if(blend == 1.0f && !mf && Alpha < 1.0f) {\n"

"       RecursionCnt++;\n"
"       payload.RecursionCnt = RecursionCnt;\n"
"       float tmin = sceneParams.TMin_TMax.x;\n"
"       float tmax = sceneParams.TMin_TMax.y;\n"
"       vec3 direction = gl_WorldRayDirectionEXT;\n"

"       if (RecursionCnt <= sceneParams.maxRecursion.x) {\n"
"           payload.hitPosition = hitPosition;\n"
"           vec3 origin = payload.hitPosition;\n"

"           traceRayEXT(\n"
"               topLevelAS,\n"
"               gl_RayFlagsCullBackFacingTrianglesEXT,\n"
"               0xff,\n"
"               clo_Index,\n"//sbtRecordOffset
"               0,\n"//sbtRecordStride
"               clo_Index,\n"//missIndex
"               origin,\n"
"               tmin,\n"
"               direction,\n"
"               tmax,\n"
"               rayPayloadEXT_location\n"//�y�C���[�h�C���f�b�N�X
"           );\n"
"       }\n"
//�A���t�@�l�̔䗦�Ō��̐F�ƌ����Փː�̐F��z��
"       ret = payload.color * (1.0f - Alpha) + difTexColor.xyz * Alpha;\n"
"    }\n"
"    return ret;\n"
"}\n";