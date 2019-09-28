//*****************************************************************************************//
//**                                                                                     **//
//**                            ShaderBasicPolygon                                       **//
//**                                                                                     **//
//*****************************************************************************************//

char* vsShaderBasicPolygon =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"

"layout (binding = 0) uniform bufferMat {\n"
"    mat4 world;\n"
"    mat4 mvp;\n"
"    mat4 bone[256];\n"
"} gBufferMat;\n"

"layout (location = 0) in vec4 inPos;\n"
"layout (location = 1) in vec3 inNormal;\n"
"layout (location = 2) in vec2 inTexCoord;\n"

"layout (location = 0) out vec3 outWpos;\n"
"layout (location = 1) out vec3 outNormal;\n"
"layout (location = 2) out vec2 outTexCoord;\n"

"void main() {\n"
    //ワールド変換行列だけ掛けた頂点,光源計算に使用
"   outWpos = (gBufferMat.world * inPos).xyz;\n"
    //法線は光源計算に使用されるのでワールド変換行列だけ掛ける
"   outNormal = normalize(mat3(gBufferMat.world) * inNormal);\n"
"   outTexCoord = inTexCoord;\n"
"   gl_Position = gBufferMat.mvp * inPos;\n"
"}\n";

char* fsShaderBasicPolygon =
"#version 450\n"
"##extension GL_ARB_separate_shader_objects : enable\n"

"layout (binding = 1) uniform sampler2D texSampler;\n"
"layout (binding = 2) uniform sampler2D norSampler;\n"
"layout (binding = 3) uniform bufferMaterial {\n"
"    vec4 diffuse;\n"
"    vec4 specular;\n"
"    vec4 ambient;\n"
"    vec4 viewPos;\n"
"    vec4 lightPos[256];\n"
"    vec4 lightColor[256];\n"
"    vec4 numLight;\n"//ライト数, 減衰1, 減衰2, 減衰3
"} gMaterial;\n"

"layout (location = 0) in vec3 inWpos;\n"
"layout (location = 1) in vec3 inNormal;\n"
"layout (location = 2) in vec2 inTexCoord;"

"layout (location = 0) out vec4 outColor;\n"

"void main() {\n"

"   vec4 NT = texture(norSampler, inTexCoord);\n"
"   vec3 norTex = normalize(inNormal * NT.xyz);\n"
"   vec4 col = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
    //ライト数だけループ
"   for(int i = 0; i < gMaterial.numLight.x; i++)\n"
"   {\n"

       ////点光源計算////
	   //対象頂点から光源までの距離計算
"      float distance = length(inWpos - gMaterial.lightPos[i].xyz);\n"
	   //↑の距離とパラメータから光の減衰率の計算    
"      float attenuation = 1.0f / \n"
"                          (gMaterial.numLight.y + \n"
"                           gMaterial.numLight.z * distance + \n"
"                           pow(gMaterial.numLight.w * distance, 2.0f));\n"
	   //ライトベクトル計算
"      vec3 lightVec = normalize(gMaterial.lightPos[i].xyz - inWpos);\n"

       ////拡散反射光計算////
       //ライトベクトルから陰影の計算
"      float lightScalar = dot(norTex,lightVec);\n"
       //値を0.0f~1.0f以内にする
"      float ScaCla = clamp(lightScalar,0.0f,1.0f);\n"

       ////鏡面反射光計算////
	   //視線ベクトル計算
"      vec3 eyeVec = normalize(gMaterial.viewPos.xyz - inWpos);\n"
       //反射計算//
"      vec3 reflect = normalize(2.0f * ScaCla * norTex - lightVec);\n"
"      float spe1 = dot(reflect, eyeVec);\n"
"      float spe2 = clamp(spe1,0.0f,1.0f);\n"
"      float spe3 = pow(spe2, 4);\n"

"      vec3 diffuse = gMaterial.diffuse.xyz * ScaCla;\n"
"      vec3 specular = gMaterial.specular.xyz * spe3;\n"

"      col.xyz += (diffuse + specular) * gMaterial.lightColor[i].xyz * attenuation;\n"
"   }\n"
"   col.xyz += gMaterial.ambient.xyz;\n"
"   vec4 tex = texture(texSampler, inTexCoord);\n"
"   outColor = tex * col;\n"
"}\n";