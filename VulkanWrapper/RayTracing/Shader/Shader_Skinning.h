//*****************************************************************************************//
//**                             Shader_Skinning.h                                       **//
//*****************************************************************************************//

char* Shader_Skinning =

"#version 460\n"
"#extension GL_EXT_ray_tracing : enable\n"
"#extension GL_EXT_buffer_reference : enable\n"
"#extension GL_EXT_scalar_block_layout : enable\n"
"#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable\n"

"layout(local_size_x = 1, local_size_y = 1) in;\n"

"struct VertexSkin {\n"
"    vec4 Position;\n"
"    vec4 Normal;\n"
"    vec4 tangent;\n"
"    vec4 TexCoord;\n"
"    vec4 SpeTexCoord;\n"
"    vec4 BoneIndex;\n"
"    vec4 BoneWeight;\n"
"};\n"
"layout(std430, set = 0, binding = 0) buffer SrcVertexBuffer {\n"
"    VertexSkin srcV[];\n" 
"};\n"

"layout(set = 0, binding = 1) uniform BoneBuffer {\n"
"    mat4 bMat[256];\n"
"} bone;\n"

"struct Vertex {\n"
"    vec4 Position;\n"
"    vec4 Normal;\n"
"    vec4 tangent;\n"
"    vec4 TexCoord;\n"
"    vec4 SpeTexCoord;\n"
"};\n"
"layout(std430, set = 0, binding = 2) buffer DstVertexBuffer { \n"
"    Vertex dstV[];\n" 
"};\n"

"void main() {\n"
"	uint index = gl_GlobalInvocationID.x;\n"

"   vec4 inPos = srcV[index].Position;\n"
"   inPos.w = 1.0f;\n"
"   vec4 inNor = srcV[index].Normal;\n"
"   vec4 inTan = srcV[index].tangent;\n"
"   vec4 boneIndex = srcV[index].BoneIndex;\n"
"   vec4 boneWeight = srcV[index].BoneWeight;\n"

"   vec4 outPos = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
"   vec4 outNor = vec4(0.0f, 0.0f, 0.0f, 0.0f);\n"
"   vec4 outTan = vec4(0.0f, 0.0f, 0.0f, 0.0f);\n"

//bone0
"   int iBone = int(boneIndex.x);\n"
"   float wei = boneWeight.x;\n"
"   mat4 m =    bone.bMat[iBone];\n"
"   outPos += m * inPos * wei;\n"
"   outNor.xyz += mat3(m) * inNor.xyz * wei;\n"
"   outTan.xyz += mat3(m) * inTan.xyz * wei;\n"
//bone1
"   iBone = int(boneIndex.y);\n"
"   wei =   boneWeight.y;\n"
"   m =     bone.bMat[iBone];\n"
"   outPos += m * inPos * wei;\n"
"   outNor.xyz += mat3(m) * inNor.xyz * wei;\n"
"   outTan.xyz += mat3(m) * inTan.xyz * wei;\n"
//bone2
"   iBone = int(boneIndex.z);\n"
"   wei =   boneWeight.z;\n"
"   m =     bone.bMat[iBone];\n"
"   outPos += m * inPos * wei;\n"
"   outNor.xyz += mat3(m) * inNor.xyz * wei;\n"
"   outTan.xyz += mat3(m) * inTan.xyz * wei;\n"
//bone3
"   iBone = int(boneIndex.w);\n"
"   wei =   boneWeight.w;\n"
"   m =     bone.bMat[iBone];\n"
"   outPos += m * inPos * wei;\n"
"   outNor.xyz += mat3(m) * inNor.xyz * wei;\n"
"   outTan.xyz += mat3(m) * inTan.xyz * wei;\n"

"   dstV[index].Position = outPos;\n"
"   dstV[index].Normal = outNor;\n"
"   dstV[index].tangent = outTan;\n"
"   dstV[index].TexCoord = srcV[index].TexCoord;\n"
"   dstV[index].SpeTexCoord = srcV[index].SpeTexCoord;\n"
"}\n";
