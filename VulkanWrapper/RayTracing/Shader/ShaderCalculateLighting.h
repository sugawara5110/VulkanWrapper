///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                ShaderCalculateLighting                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////////////

char* ShaderCalculateLighting =
//////////////////////////////////////���C�g�v�Z/////////////////////////////////////////////////////////
"struct LightOut\n"
"{\n"
"    vec3 Diffuse;\n"
"    vec3 Speculer;\n"
"};\n"

"LightOut LightCom(vec3 SpeculerCol, vec3 Diffuse, vec3 Ambient, vec3 Nor, \n"
"                  vec3 wPos, vec3 lightCol, vec3 eyePos, vec3 lightVec, float distAtten, float shininess)\n"
"{\n"
//�o�͗p
"    LightOut Out = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"

//���C�g�x�N�g�����K��
"    vec3 LVec = normalize(lightVec);\n"
//�p�x�������f�B�t�F�[�Y
"    float angleAttenDif = clamp(dot(LVec, Nor), 0.0f, 1.0f);"

//�����x�N�g��
"    vec3 eyeVec = normalize(eyePos - wPos);\n"
//���˃x�N�g��
"    vec3 reflectVec = reflect(-LVec, Nor);\n"
//�p�x�������X�y�L����
"    float angleAttenSpe = pow(clamp(dot(eyeVec, reflectVec), 0.0f, 1.0f), shininess);\n"

//�f�B�t�F�[�Y�o��
"    Out.Diffuse = distAtten * lightCol * (angleAttenDif * Diffuse + Ambient);\n"
//�X�y�L�����o��
"    Out.Speculer = distAtten * lightCol * angleAttenSpe * SpeculerCol;\n"
"    return Out;\n"
"}\n"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////�|�C���g���C�g�v�Z(���C�g�������[�v�����Ďg�p����)////////////////////////////////
"LightOut PointLightCom(vec3 SpeculerCol, vec3 Diffuse, vec3 Ambient, vec3 Nor, \n"
"                       vec4 lightPos, vec3 wPos, vec4 lightSt, vec3 lightCol, vec3 eyePos, float shininess)\n"
"{\n"
//�o�͗p
"    LightOut Out = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"

//���C�g�x�N�g��
"    vec3 lightVec = lightPos.xyz - wPos;\n"

//���_��������܂ł̋������v�Z
"    float distance = length(lightVec);\n"

//���C�g�I�t, �����W���O�͔�΂�
"    if (lightPos.w == 1.0f && distance < lightSt.x){\n"

//����������         
"       float distAtten = 1.0f / \n"
"                        (lightSt.y + \n"
"                         lightSt.z * distance + \n"
"                         lightSt.w * distance * distance);\n"

//�����v�Z
"       Out = LightCom(SpeculerCol, Diffuse, Ambient, Nor, \n"
"                      wPos, lightCol, eyePos, lightVec, distAtten, shininess);\n"

"    }\n"
"    return Out;\n"
"}\n"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////���s�����v�Z/////////////////////////////////////////////////////////
"LightOut DirectionalLightCom(vec3 SpeculerCol, vec3 Diffuse, vec3 Ambient, vec3 Nor, \n"
"                             vec4 DlightSt, vec3 Dir, vec3 DCol, vec3 wPos, vec3 eyePos, float shininess)\n"
"{\n"
//�o�͗p
"    LightOut Out = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"

"    if(DlightSt.x == 1.0f)\n"
"    {\n"

//�����v�Z
"       Out = LightCom(SpeculerCol, Diffuse, Ambient, Nor, \n"
"                      wPos, DCol, eyePos, -Dir, 1.0f, shininess);\n"

"    }\n"
"    return Out;\n"
"}\n";
////////////////////////////////////////////////////////////////////////////////////////////////////////