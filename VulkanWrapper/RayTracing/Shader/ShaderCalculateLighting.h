///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                ShaderCalculateLighting                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////////////

char* ShaderCalculateLighting =
//////////////////////////////////////ライト計算/////////////////////////////////////////////////////////
"struct LightOut\n"
"{\n"
"    vec3 Diffuse;\n"
"    vec3 Speculer;\n"
"};\n"

"LightOut LightCom(vec3 SpeculerCol, vec3 Diffuse, vec3 Ambient, vec3 Nor, \n"
"                  vec3 wPos, vec3 lightCol, vec3 eyePos, vec3 lightVec, float distAtten, float shininess)\n"
"{\n"
//出力用
"    LightOut Out = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"

//ライトベクトル正規化
"    vec3 LVec = normalize(lightVec);\n"
//角度減衰率ディフェーズ
"    float angleAttenDif = clamp(dot(LVec, Nor), 0.0f, 1.0f);"

//視線ベクトル
"    vec3 eyeVec = normalize(eyePos - wPos);\n"
//反射ベクトル
"    vec3 reflectVec = reflect(-LVec, Nor);\n"
//角度減衰率スペキュラ
"    float angleAttenSpe = pow(clamp(dot(eyeVec, reflectVec), 0.0f, 1.0f), shininess);\n"

//ディフェーズ出力
"    Out.Diffuse = distAtten * lightCol * (angleAttenDif * Diffuse + Ambient);\n"
//スペキュラ出力
"    Out.Speculer = distAtten * lightCol * angleAttenSpe * SpeculerCol;\n"
"    return Out;\n"
"}\n"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////ポイントライト計算(ライト数分ループさせて使用する)////////////////////////////////
"LightOut PointLightCom(vec3 SpeculerCol, vec3 Diffuse, vec3 Ambient, vec3 Nor, \n"
"                       vec4 lightPos, vec3 wPos, vec4 lightSt, vec3 lightCol, vec3 eyePos, float shininess)\n"
"{\n"
//出力用
"    LightOut Out = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"

//ライトベクトル
"    vec3 lightVec = lightPos.xyz - wPos;\n"

//頂点から光源までの距離を計算
"    float distance = length(lightVec);\n"

//ライトオフ, レンジより外は飛ばす
"    if (lightPos.w == 1.0f && distance < lightSt.x){\n"

//距離減衰率         
"       float distAtten = 1.0f / \n"
"                        (lightSt.y + \n"
"                         lightSt.z * distance + \n"
"                         lightSt.w * distance * distance);\n"

//光源計算
"       Out = LightCom(SpeculerCol, Diffuse, Ambient, Nor, \n"
"                      wPos, lightCol, eyePos, lightVec, distAtten, shininess);\n"

"    }\n"
"    return Out;\n"
"}\n"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////平行光源計算/////////////////////////////////////////////////////////
"LightOut DirectionalLightCom(vec3 SpeculerCol, vec3 Diffuse, vec3 Ambient, vec3 Nor, \n"
"                             vec4 DlightSt, vec3 Dir, vec3 DCol, vec3 wPos, vec3 eyePos, float shininess)\n"
"{\n"
//出力用
"    LightOut Out = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"

"    if(DlightSt.x == 1.0f)\n"
"    {\n"

//光源計算
"       Out = LightCom(SpeculerCol, Diffuse, Ambient, Nor, \n"
"                      wPos, DCol, eyePos, -Dir, 1.0f, shininess);\n"

"    }\n"
"    return Out;\n"
"}\n";
////////////////////////////////////////////////////////////////////////////////////////////////////////