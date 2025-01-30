//*****************************************************************************************//
//**                                                                                     **//
//**                            ShaderBasicPolygon                                       **//
//**                                                                                     **//
//*****************************************************************************************//

char* vsShaderBasicPolygon =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"

"layout (binding = 0, set = 0) uniform bufferMatVP {\n"
"    mat4 ViewProjection;\n"
"} gBufferMatVp;\n"

"struct Instancing{\n"
"    mat4 world;\n"
"    vec4 pXpYmXmY;\n"
"    vec4 d1;\n"
"    vec4 d2;\n"
"    vec4 d3;\n"
"};\n"

"layout (binding = 1, set = 0) uniform bufferMat {\n"
"    Instancing ins[replace_NUM_Ins_CB];\n"
"} gBufferMat;\n"

"layout (binding = 0, set = 1) uniform bufferMat_bone {\n"
"    mat4 bone[replace_NUM_BONE_CB];\n"
"} gBufferMat_bone;\n"

"layout (location = 0) in vec4 inPos;\n"
"layout (location = 1) in vec3 inNormal;\n"
"layout (location = 2) in vec2 inTexCoord;\n"
"layout (location = 3) in vec2 inSpeTexCoord;\n"

"layout (location = 0) out vec3 outWpos;\n"
"layout (location = 1) out vec3 outNormal;\n"
"layout (location = 2) out vec2 outTexCoord;\n"
"layout (location = 3) out vec2 outSpeTexCoord;\n"

"void main() {\n"
"   mat4 world = gBufferMat.ins[gl_InstanceIndex].world;\n"
    //ワールド変換行列だけ掛けた頂点,光源計算に使用
"   outWpos = (world * inPos).xyz;\n"
    //法線は光源計算に使用されるのでワールド変換行列だけ掛ける
"   outNormal = normalize(mat3(world) * inNormal);\n"

"   vec4 pXpYmXmY = gBufferMat.ins[gl_InstanceIndex].pXpYmXmY;\n"

"   outTexCoord.x = inTexCoord.x * pXpYmXmY.x + pXpYmXmY.x * pXpYmXmY.z;\n"
"   outTexCoord.y = inTexCoord.y * pXpYmXmY.y + pXpYmXmY.y * pXpYmXmY.w;\n" 

"   outSpeTexCoord.x = inSpeTexCoord.x * pXpYmXmY.x + pXpYmXmY.x * pXpYmXmY.z;\n"
"   outSpeTexCoord.y = inSpeTexCoord.y * pXpYmXmY.y + pXpYmXmY.y * pXpYmXmY.w;\n" 

"   mat4 mvp = gBufferMatVp.ViewProjection * world;\n"
"   gl_Position = mvp * inPos;\n"//directxと逆
"}\n";

char* fsShaderBasicPolygon =
"#extension GL_ARB_separate_shader_objects : enable\n"

"layout (binding = 0, set = 2) uniform sampler2D texSampler;\n"
"layout (binding = 1, set = 2) uniform sampler2D norSampler;\n"
"layout (binding = 2, set = 2) uniform sampler2D speSampler;\n"
"layout (binding = 0, set = 3) uniform bufferMaterial {\n"
"    vec4 diffuse;\n"
"    vec4 specular;\n"
"    vec4 ambient;\n"
"    vec4 viewPos;\n"
"    vec4 numLight;\n"//ライト数, 減衰1, 減衰2, 減衰3
"    vec4 uvSwitch;\n"
"    vec4 d1;\n"
"    vec4 d2;\n"
"} gMaterial;\n"

"struct Light{\n"
"    vec4 lightPos;\n"
"    vec4 lightColor;\n"
"    vec4 d1;\n"
"    vec4 d2;\n"
"};\n"

"layout (binding = 1, set = 3) uniform LightCB {\n"
"    Light light[replace_NUM_Light_CB];\n"
"} LightCb;\n"

"layout (location = 0) in vec3 inWpos;\n"
"layout (location = 1) in vec3 inNormal;\n"
"layout (location = 2) in vec2 inTexCoord;\n"
"layout (location = 3) in vec2 inSpeTexCoord;\n"

"layout (location = 0) out vec4 outColor;\n"

"void main() {\n"
    //uv切り替え
"   vec2 texCoord;\n"
"   vec2 speTexCoord;\n"
"   if(gMaterial.uvSwitch.x == 0.0f)\n"//切り替え無
"   {\n"
"      texCoord = inTexCoord;\n"
"      speTexCoord = inSpeTexCoord;\n"
"   }\n"
"   if(gMaterial.uvSwitch.x == 1.0f)\n"//逆転
"   {\n"
"      texCoord = inSpeTexCoord;\n"
"      speTexCoord = inTexCoord;\n"
"   }\n"
"   if(gMaterial.uvSwitch.x == 2.0f)\n"//どちらもuv0
"   {\n"
"      texCoord = inTexCoord;\n"
"      speTexCoord = inTexCoord;\n"
"   }\n"
"   if(gMaterial.uvSwitch.x == 3.0f)\n"//どちらもuv1
"   {\n"
"      texCoord = inSpeTexCoord;\n"
"      speTexCoord = inSpeTexCoord;\n"
"   }\n"

"   vec4 NT = texture(norSampler, texCoord);\n"
"   vec3 norTex = normalize(inNormal * NT.xyz);\n"

"   LightOut emissiveColor = LightOut(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));\n"
"   LightOut Out;\n"

    //ライト数だけループ
"   for(int i = 0; i < gMaterial.numLight.x; i++)\n"
"   {\n"

"      vec3 lightPos = LightCb.light[i].lightPos.xyz;\n"
"      vec4 emPos = vec4(lightPos, 1.0f);\n"//1.0f ライトONのまま

"      float distance = length(inWpos - lightPos);\n"
"      vec4 lightst = gMaterial.numLight;\n"
"      lightst.x = distance * 2.0f;\n"//常にレンジ内
"      vec3 lightColor = LightCb.light[i].lightColor.xyz;\n"
"      float shininess = 4.0f;\n"

"      Out = PointLightCom(gMaterial.specular.xyz, gMaterial.diffuse.xyz, gMaterial.ambient.xyz, norTex, emPos, \n"
"                          inWpos, lightst, lightColor, gMaterial.viewPos.xyz, shininess);\n"

"      emissiveColor.Diffuse += Out.Diffuse;\n"
"      emissiveColor.Speculer += Out.Speculer;\n"

"   }\n"
"   vec4 difCol = vec4(emissiveColor.Diffuse, 1.0f);\n"
"   vec4 speCol = vec4(emissiveColor.Speculer, 1.0f);\n"
"   vec4 dTex = texture(texSampler, texCoord);\n"
"   vec4 sTex = texture(speSampler, speTexCoord);\n"
"   outColor = dTex * difCol + sTex * speCol;\n"
"}\n";