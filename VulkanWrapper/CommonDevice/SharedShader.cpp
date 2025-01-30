///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                SharedShader.cpp                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "SharedShader.h"

static char* ShaderCalculateLighting =
//////////////////////////////////////���C�g�v�ZOutPut/////////////////////////////////////////////////////
"struct LightOut\n"
"{\n"
"    vec3 Diffuse;\n"
"    vec3 Speculer;\n"
"};\n"
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////�����o�[�g�g�U����/////////////////////////////////////////////////
"vec3 Lambert(float distAtten, vec3 lightCol, vec3 Diffuse, vec3 Ambient, vec3 Nor, vec3 LVec)\n"
"{\n"
     //�p�x�������f�B�t�F�[�Y
     //���ςŌv�Z����̂Ō����̃x�N�g�����t�ɂ���
"    float angleAttenDif = clamp(dot(-LVec, Nor), 0.0f, 1.0f);\n"

"    return distAtten * lightCol * angleAttenDif * Diffuse + Ambient;\n"
"}\n"
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////�t�H�����ʔ���/////////////////////////////////////////////////////
"vec3 Phong(float distAtten, vec3 lightCol, vec3 SpeculerCol, vec3 Nor, vec3 LVec,\n"
"           vec3 wPos, vec3 eyePos, float shininess)\n"
"{\n"
//�����x�N�g��
"    vec3 eyeVec = normalize(wPos - eyePos);\n"
//���˃x�N�g��
"    vec3 reflectVec = normalize(reflect(LVec, Nor));\n"
//�p�x�������X�y�L����
"    float angleAttenSpe = pow(clamp(dot(reflectVec, -eyeVec), 0.0f, 1.0f), shininess);\n"

"    return distAtten * lightCol * angleAttenSpe * SpeculerCol;\n"
"}\n"
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////���C�g�v�Z/////////////////////////////////////////////////////////
"LightOut LightCom(vec3 SpeculerCol, vec3 Diffuse, vec3 Ambient, vec3 Nor,\n"
"                  vec3 wPos, vec3 lightCol, vec3 eyePos, vec3 lightVec, float distAtten, float shininess)\n"
"{\n"
//�o�͗p
"    LightOut Out = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"

//���C�g�x�N�g�����K��
"    vec3 LVec = normalize(lightVec);\n"

//�f�B�t�F�[�Y�o��
"    Out.Diffuse = Lambert(distAtten, lightCol, Diffuse, Ambient, Nor, LVec);\n"
//�X�y�L�����o��
"    Out.Speculer = Phong(distAtten, lightCol, SpeculerCol, Nor, LVec, wPos, eyePos, shininess);\n"
"    return Out;\n"
"}\n"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////�|�C���g���C�g�v�Z(���C�g�������[�v�����Ďg�p����)////////////////////////////////
"LightOut PointLightCom(vec3 SpeculerCol, vec3 Diffuse, vec3 Ambient, vec3 Nor,\n"
"                       vec4 lightPos, vec3 wPos, vec4 lightSt, vec3 lightCol, vec3 eyePos, float shininess)\n"
"{\n"
//�o�͗p
"    LightOut Out = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"

//���C�g�x�N�g�� (���_�̈ʒu - �_�����̈ʒu)
"    vec3 lightVec = wPos - lightPos.xyz;\n"

//���_��������܂ł̋������v�Z
"    float distance = length(lightVec);\n"

//���C�g�I�t, �����W���O�͔�΂�
"    if (lightPos.w == 1.0f && distance < lightSt.x)\n"
"    {\n"

//����������         
"        float distAtten = 1.0f / \n"
"              (lightSt.y + \n"
"               lightSt.z * distance + \n"
"               lightSt.w * distance * distance);\n"

//�����v�Z
"        Out = LightCom(SpeculerCol, Diffuse, Ambient, Nor,\n"
"                       wPos, lightCol, eyePos, lightVec, distAtten, shininess);\n"

"    }\n"
"    return Out;\n"
"}\n"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////�|�C���g���C�g�v�Z, ������������///////////////////////////////////////////////////
"LightOut PointLightComNoDistance(vec3 SpeculerCol, vec3 Diffuse, vec3 Ambient, vec3 Nor,\n"
"                                 vec4 lightPos, vec3 wPos, vec3 lightCol, vec3 eyePos, float shininess)\n"
"{\n"
//���C�g�x�N�g�� (���_�̈ʒu - �_�����̈ʒu)
"    vec3 lightVec = wPos - lightPos.xyz;\n"

//����������         
"    float distAtten = 1.0f;\n"

//�����v�Z
"    return LightCom(SpeculerCol, Diffuse, Ambient, Nor,\n"
"                    wPos, lightCol, eyePos, lightVec, distAtten, shininess);\n"
"}\n"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////���s�����v�Z/////////////////////////////////////////////////////////
"LightOut DirectionalLightCom(vec3 SpeculerCol, vec3 Diffuse, vec3 Ambient, vec3 Nor,\n"
"                             vec4 DlightSt, vec3 Dir, vec3 DCol, vec3 wPos, vec3 eyePos, float shininess)\n"
"{\n"
//�o�͗p
"    LightOut Out = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"

"    if (DlightSt.x == 1.0f)\n"
"    {\n"

//�����v�Z
"        Out = LightCom(SpeculerCol, Diffuse, Ambient, Nor,\n"
"                       wPos, DCol, eyePos, Dir, 1.0f, shininess);\n"

"    }\n"
"    return Out;\n"
"}\n";
////////////////////////////////////////////////////////////////////////////////////////////////////////

char* SharedShader::getShaderCalculateLighting() {
    return ShaderCalculateLighting;
}