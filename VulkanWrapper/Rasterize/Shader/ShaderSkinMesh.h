//*****************************************************************************************//
//**                                                                                     **//
//**                             ShaderSkinMesh                                          **//
//**                                                                                     **//
//*****************************************************************************************//

char* vsShaderSkinMesh =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"

"layout (binding = 0, set = 0) uniform bufferMatVP {\n"
"    mat4 ViewProjection;\n"
"} gBufferMatVp;\n"

"struct Instancing{\n"
"    mat4 world;\n"
"    vec4 pXpYmXmY;\n"
"    vec4 addCol;\n"
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
"layout (location = 4) in vec4 boneIndex;\n"
"layout (location = 5) in vec4 boneWeight;\n"

"layout (location = 0) out vec3 outWpos;\n"
"layout (location = 1) out vec3 outNormal;\n"
"layout (location = 2) out vec2 outTexCoord;\n"
"layout (location = 3) out vec2 outSpeTexCoord;\n"
"layout (location = 4) out vec4 outAddCol;\n"

"void main() {\n"
//スキニング
"   vec4 sInPos = inPos;\n"
"   vec3 sInNor = inNormal;\n"
"   vec4 sOutPos = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
"   vec3 sOutNor = vec3(0.0f, 0.0f, 0.0f);\n"

//bone0
"   int iBone = int(boneIndex.x);\n"//1個目のBoneIndex取り出し
"   float wei = boneWeight.x;\n"//1個目のBoneWeight取り出し
"   mat4 m = gBufferMat_bone.bone[iBone];\n"//姿勢行列配列からiBone番目行列取り出し
"   sOutPos += m * sInPos * wei;\n"//スキニング後頂点 = 姿勢行列 * 頂点 * 頂点ウエイト(DirectXとは逆)
"   sOutNor += mat3(m) * sInNor * wei;\n"//スキニング後法線 = 姿勢行列 * 法線 * 頂点ウエイト
//bone1
"   iBone = int(boneIndex.y);\n"
"   wei = boneWeight.y;\n"
"   m = gBufferMat_bone.bone[iBone];\n"
"   sOutPos += m * sInPos * wei;\n"
"   sOutNor += mat3(m) * sInNor * wei;\n"
//bone2
"   iBone = int(boneIndex.z);\n"
"   wei = boneWeight.z;\n"
"   m = gBufferMat_bone.bone[iBone];\n"
"   sOutPos += m * sInPos * wei;\n"
"   sOutNor += mat3(m) * sInNor * wei;\n"
//bone3
"   iBone = int(boneIndex.w);\n"
"   wei = boneWeight.w;\n"
"   m = gBufferMat_bone.bone[iBone];\n"
"   sOutPos += m * sInPos * wei;\n"
"   sOutNor += mat3(m) * sInNor * wei;\n"

"   mat4 world = gBufferMat.ins[gl_InstanceIndex].world;\n"
"   outAddCol = gBufferMat.ins[gl_InstanceIndex].addCol;\n"
//ワールド変換行列だけ掛けた頂点,光源計算に使用
"   outWpos = (world * sOutPos).xyz;\n"
//法線は光源計算に使用されるのでワールド変換行列だけ掛ける
"   outNormal = normalize(mat3(world) * sOutNor);\n"
"   outTexCoord = inTexCoord;\n"
"   outSpeTexCoord = inSpeTexCoord;\n"

"   mat4 mvp = gBufferMatVp.ViewProjection * world;\n"
"   gl_Position = mvp * sOutPos;\n"
"}\n";