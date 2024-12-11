//*****************************************************************************************//
//**                             Shader_traceRay_OneRay.h                                **//
//*****************************************************************************************//

char* Shader_traceRay_OneRay =

///////////////////////�����֌������΂�, �q�b�g�����ꍇ���邳�����Z//////////////////////////
"vec3 EmissivePayloadCalculate(in int RecursionCnt, in vec3 hitPosition, \n"
"                              in vec3 difTexColor, in vec3 speTexColor, in vec3 normal)\n"
"{\n"
"    MaterialCB mcb = matCB[gl_InstanceID];\n"
"    int mNo = int(mcb.materialNo.x);\n"
"    vec3 ret = difTexColor;\n"

"    bool mf = materialIdent(mNo, EMISSIVE);\n"
"    if(!mf) {\n"//emissive�ȊO

"       LightOut emissiveColor = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"
"       LightOut Out;\n"

"       vec3 SpeculerCol = mcb.Speculer.xyz;\n"
"       vec3 Diffuse = mcb.Diffuse.xyz;\n"
"       vec3 Ambient = mcb.Ambient.xyz + sceneParams.GlobalAmbientColor.xyz;\n"
"       float shininess = mcb.shininess.x;\n"

"       for(int i = 0; i < sceneParams.numEmissive.x; i++) {\n"

"           vec4 emissivePosition = sceneParams.emissivePosition[i];\n"

"           if(emissivePosition.w == 1.0f) {\n"

"              vec3 lightVec = normalize(emissivePosition.xyz - hitPosition);\n"
"              vec3 direction = lightVec;\n"
"              payload.pLightID = int(sceneParams.emissiveNo[i].x);\n"
"              payload.hitPosition = hitPosition;\n"
"              payload.mNo = EMISSIVE; \n"//��������p

"              traceRay(RecursionCnt,\n"
"                       gl_RayFlagsCullBackFacingTrianglesEXT,\n"
"                       1,\n"
"                       1,\n"
"                       direction);\n"

"              Out = PointLightCom(SpeculerCol, Diffuse, Ambient, normal, emissivePosition, \n"
"                                  hitPosition, payload.lightst, payload.color, sceneParams.cameraPosition.xyz, shininess);\n"

"              emissiveColor.Diffuse += Out.Diffuse;\n"
"              emissiveColor.Speculer += Out.Speculer;\n"
"           }\n"
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
"                              in vec3 difTexColor, in vec3 normal, inout int hitInstanceId)\n"
"{\n"
"    int mNo = int(matCB[gl_InstanceID].materialNo.x);\n"
"    vec3 ret = difTexColor;\n"

"    hitInstanceId = gl_InstanceID; \n"//���g��ID��������

"    if(materialIdent(mNo, METALLIC)) {\n"//METALLIC

"       vec3 eyeVec = gl_WorldRayDirectionEXT;\n"
//���˃x�N�g��
"       vec3 reflectVec = reflect(eyeVec, normalize(normal));\n"
"       vec3 direction = reflectVec;\n"//���˕�����Ray���΂�

"       payload.hitPosition = hitPosition; \n"

"       traceRay(RecursionCnt,\n"
"                gl_RayFlagsCullBackFacingTrianglesEXT,\n"
"                0,\n"
"                0,\n"
"                direction);\n"

"       vec3 outCol = vec3(0.0f, 0.0f, 0.0f);\n"
"       if (payload.hit) {\n"
"           outCol = difTexColor * payload.color;\n"//�q�b�g�����ꍇ�f�荞�݂Ƃ��ď�Z
"           hitInstanceId = payload.hitInstanceId;\n"//�q�b�g����ID��������
"           int hitmNo = payload.mNo;\n"
"           if(materialIdent(hitmNo, EMISSIVE)){\n"
"              outCol = payload.color;\n"
"           }\n"
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

"       float in_eta = AIR_RefractiveIndex;\n"
"       float out_eta = mcb.RefractiveIndex.x;\n"

"       vec3 r_eyeVec = -gl_WorldRayDirectionEXT;\n"
"       float norDir = dot(r_eyeVec, normal);\n"
"       if (norDir < 0.0f)\n"
"       {\n"
"           normal *= -1.0f;\n"
"           in_eta = mcb.RefractiveIndex.x;\n"
"           out_eta = AIR_RefractiveIndex;\n"
"       }\n"

"       vec3 eyeVec = gl_WorldRayDirectionEXT;\n"
"       float eta = in_eta / out_eta;\n"
"       vec3 direction = refract(eyeVec, normalize(normal), eta);\n"

"       payload.hitPosition = hitPosition;\n"

"       traceRay(RecursionCnt,\n"
"                gl_RayFlagsCullBackFacingTrianglesEXT,\n"
"                0,\n"
"                0,\n"
"                direction);\n"

//�A���t�@�l�̔䗦�Ō��̐F�ƌ����Փː�̐F��z��
"       ret = payload.color * (1.0f - Alpha) + difTexColor.xyz * Alpha;\n"
"    }\n"
"    return ret;\n"
"}\n"

////////////////////////////////////////ONE_RAY//////////////////////////////////////////
"vec3 PayloadCalculate_OneRay(in int RecursionCnt, in vec3 hitPosition,\n"
"                             in vec4 difTex, in vec3 speTex, in vec3 normalMap,\n"
"                             inout int hitInstanceId)\n"
"{\n"
//�����ւ̌���
"    difTex.xyz = EmissivePayloadCalculate(RecursionCnt, hitPosition, difTex.xyz, speTex, normalMap);\n"
//���˕����ւ̌���
"    difTex.xyz = MetallicPayloadCalculate(RecursionCnt, hitPosition, difTex.xyz, normalMap, hitInstanceId);\n"
//������
"    difTex.xyz = Translucent(RecursionCnt, hitPosition, difTex, normalMap);\n"

"    return difTex.xyz;\n"
"}\n";