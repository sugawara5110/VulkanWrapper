///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                SharedShader.cpp                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "SharedShader.h"

static char* ShaderCalculateLighting =
//////////////////////////////////////ライト計算OutPut/////////////////////////////////////////////////////
"struct LightOut\n"
"{\n"
"    vec3 Diffuse;\n"
"    vec3 Speculer;\n"
"};\n"
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////ランバート拡散反射/////////////////////////////////////////////////
"vec3 Lambert(float distAtten, vec3 lightCol, vec3 Diffuse, vec3 Ambient, vec3 Nor, vec3 LVec)\n"
"{\n"
     //角度減衰率ディフェーズ
     //内積で計算するので光源のベクトルを逆にする
"    float angleAttenDif = clamp(dot(-LVec, Nor), 0.0f, 1.0f);\n"

"    return distAtten * lightCol * angleAttenDif * Diffuse + Ambient;\n"
"}\n"
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////フォン鏡面反射/////////////////////////////////////////////////////
"vec3 Phong(float distAtten, vec3 lightCol, vec3 SpeculerCol, vec3 Nor, vec3 LVec,\n"
"           vec3 wPos, vec3 eyePos, float shininess)\n"
"{\n"
//視線ベクトル
"    vec3 eyeVec = normalize(wPos - eyePos);\n"
//反射ベクトル
"    vec3 reflectVec = normalize(reflect(LVec, Nor));\n"
//角度減衰率スペキュラ
"    float angleAttenSpe = pow(clamp(dot(reflectVec, -eyeVec), 0.0f, 1.0f), shininess);\n"

"    return distAtten * lightCol * angleAttenSpe * SpeculerCol;\n"
"}\n"
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////ライト計算/////////////////////////////////////////////////////////
"LightOut LightCom(vec3 SpeculerCol, vec3 Diffuse, vec3 Ambient, vec3 Nor,\n"
"                  vec3 wPos, vec3 lightCol, vec3 eyePos, vec3 lightVec, float distAtten, float shininess)\n"
"{\n"
//出力用
"    LightOut Out = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"

//ライトベクトル正規化
"    vec3 LVec = normalize(lightVec);\n"

//ディフェーズ出力
"    Out.Diffuse = Lambert(distAtten, lightCol, Diffuse, Ambient, Nor, LVec);\n"
//スペキュラ出力
"    Out.Speculer = Phong(distAtten, lightCol, SpeculerCol, Nor, LVec, wPos, eyePos, shininess);\n"
"    return Out;\n"
"}\n"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////ポイントライト計算(ライト数分ループさせて使用する)////////////////////////////////
"LightOut PointLightCom(vec3 SpeculerCol, vec3 Diffuse, vec3 Ambient, vec3 Nor,\n"
"                       vec4 lightPos, vec3 wPos, vec4 lightSt, vec3 lightCol, vec3 eyePos, float shininess)\n"
"{\n"
//出力用
"    LightOut Out = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"

//ライトベクトル (頂点の位置 - 点光源の位置)
"    vec3 lightVec = wPos - lightPos.xyz;\n"

//頂点から光源までの距離を計算
"    float distance = length(lightVec);\n"

//ライトオフ, レンジより外は飛ばす
"    if (lightPos.w == 1.0f && distance < lightSt.x)\n"
"    {\n"

//距離減衰率         
"        float distAtten = 1.0f / \n"
"              (lightSt.y + \n"
"               lightSt.z * distance + \n"
"               lightSt.w * distance * distance);\n"

//光源計算
"        Out = LightCom(SpeculerCol, Diffuse, Ambient, Nor,\n"
"                       wPos, lightCol, eyePos, lightVec, distAtten, shininess);\n"

"    }\n"
"    return Out;\n"
"}\n"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////ポイントライト計算, 距離減衰無し///////////////////////////////////////////////////
"LightOut PointLightComNoDistance(vec3 SpeculerCol, vec3 Diffuse, vec3 Ambient, vec3 Nor,\n"
"                                 vec4 lightPos, vec3 wPos, vec3 lightCol, vec3 eyePos, float shininess)\n"
"{\n"
//ライトベクトル (頂点の位置 - 点光源の位置)
"    vec3 lightVec = wPos - lightPos.xyz;\n"

//距離減衰率         
"    float distAtten = 1.0f;\n"

//光源計算
"    return LightCom(SpeculerCol, Diffuse, Ambient, Nor,\n"
"                    wPos, lightCol, eyePos, lightVec, distAtten, shininess);\n"
"}\n"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////平行光源計算/////////////////////////////////////////////////////////
"LightOut DirectionalLightCom(vec3 SpeculerCol, vec3 Diffuse, vec3 Ambient, vec3 Nor,\n"
"                             vec4 DlightSt, vec3 Dir, vec3 DCol, vec3 wPos, vec3 eyePos, float shininess)\n"
"{\n"
//出力用
"    LightOut Out = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"

"    if (DlightSt.x == 1.0f)\n"
"    {\n"

//光源計算
"        Out = LightCom(SpeculerCol, Diffuse, Ambient, Nor,\n"
"                       wPos, DCol, eyePos, Dir, 1.0f, shininess);\n"

"    }\n"
"    return Out;\n"
"}\n";
////////////////////////////////////////////////////////////////////////////////////////////////////////

char* SharedShader::getShaderCalculateLighting() {
    return ShaderCalculateLighting;
}