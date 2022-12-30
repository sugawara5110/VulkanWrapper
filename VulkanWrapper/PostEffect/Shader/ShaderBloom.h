///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                           ShaderBloom.h                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////////////

char* ShaderBloom_Com =

"#version 460\n"
"#extension GL_EXT_nonuniform_qualifier : enable\n"

"layout(local_size_x = 1, local_size_y = 1) in;\n"

"layout(binding = 0, set = 0) uniform BloomParam {\n"
"    float GaussianWid;\n"//�K�E�X��
"    float bloomStrength;\n"//�u���[������
"    float thresholdLuminance;\n"//�P�x臒l
"    float numGaussFilter;\n"//�K�E�X�t�B���^�[��
"    int   InstanceID;\n"//�����Ώ�ID
"} bloomParam;\n";

char* ShaderBloom0 =

"layout(binding = 1, set = 0, rgba8) uniform image2D image;\n"//�����_�����O������̉摜
"layout(binding = 2, set = 0, r32f) uniform image2D InstanceIdMap;\n"
"layout(binding = 3, set = 0, rgba8) uniform image2D outLuminance;\n"

"void getLuminanceUV(in uvec2 dtid, out vec2 uv)\n"
"{\n"
"   ivec2 size = imageSize(outLuminance);\n"
"   uv.x = float(dtid.x) / float(size.x);\n"
"   uv.y = float(dtid.y) / float(size.y);\n"
"}\n"

//�u���[���Ώۂ�InstanceID���ɋP�x�𒊏o
"void main()\n"
"{\n"
"	uvec2 index = gl_GlobalInvocationID.xy;\n"//outLuminance�̃T�C�Y�ł̈ʒu
"   vec2 uv;\n"
"   getLuminanceUV(index, uv);\n"//outLuminance��uv

"   ivec2 isize = imageSize(image);\n"//�����_�����O������̉摜�T�C�Y
"   vec2 fsize;\n"
"   fsize.x = float(isize.x) * uv.x;\n"//�����_�����O�摜�T�C�Y��uv�v�Z(InstanceIdMap�Ɠ���)
"   fsize.y = float(isize.y) * uv.y;\n"

"   vec4 L = imageLoad(image, ivec2(fsize));\n"//�����_�����O������̉摜
"   L.w = 0.0f;\n"

"   vec4 map = imageLoad(InstanceIdMap, ivec2(fsize));\n"
"   int instanceId = int(map.x);\n"//�}�b�v���ID�ǂݍ���

"   if(L.x + L.y + L.z > bloomParam.thresholdLuminance * 3.0f && \n"//臒l��荂����
"      bloomParam.InstanceID == instanceId)\n"//�Ώ�ID�ƈ�v���邩
"   {\n"
"     imageStore(outLuminance, ivec2(index), L);\n"//��v�����ꍇ��������,(����Ń}�b�v�͈͂����������܂��)
"   }else\n"
"   {\n"
"     imageStore(outLuminance, ivec2(index), vec4(0.0f ,0.0f ,0.0f ,0.0f));\n"//��v���Ȃ��ꍇ�̓[��
"   }\n"
"}\n";

char* ShaderBloom1 =

"layout(std430, binding = 1, set = 0) buffer GaussianFilter { float gaussianFilter[]; };\n"
"layout(binding = 2, set = 0, rgba8) uniform image2D inLuminance;\n"
"layout(binding = 3, set = 0, rgba8) uniform image2D outBloom0;\n"

//�u���[����
"void main()\n"
"{\n"
"	uvec2 index = gl_GlobalInvocationID.xy;\n"//outLuminance�̃T�C�Y�ł̈ʒu

"   int halfwid = int(bloomParam.GaussianWid);\n"
"   vec3 col = vec3(0.0f, 0.0f, 0.0f);\n"

"   for(int i = 0; i < halfwid; i++){\n"
"     uvec2 ind;\n"
"     ind.x = index.x + i;\n"
"     ind.y = index.y;\n"
"     vec4 L = imageLoad(inLuminance, ivec2(ind));\n"
"     col += L.xyz * gaussianFilter[i];\n"
"     ind.x = index.x - i;\n"
"     ind.y = index.y;\n"
"     L = imageLoad(inLuminance, ivec2(ind));\n"
"     col += L.xyz * gaussianFilter[i];\n"
"   }\n"

"   vec4 Out = vec4(0.0f, 0.0f, 0.0f, 0.0f);\n"
"   Out.xyz = col;\n"
"   imageStore(outBloom0, ivec2(index), Out);\n"
"}\n";

char* ShaderBloom2 =

"layout(std430, binding = 1, set = 0) buffer GaussianFilter { float gaussianFilter[]; };\n"
"layout(binding = 2, set = 0, rgba8) uniform image2D inBloom0;\n"
"layout(binding = 3, set = 0, rgba8) uniform image2D outBloom1;\n"

//�u���[���c
"void main()\n"
"{\n"
"	uvec2 index = gl_GlobalInvocationID.xy;\n"//outLuminance�̃T�C�Y�ł̈ʒu

"   int halfwid = int(bloomParam.GaussianWid);\n"
"   vec3 col = vec3(0.0f, 0.0f, 0.0f);\n"

"   for(int i = 0; i < halfwid; i++){\n"
"     uvec2 ind;\n"
"     ind.x = index.x;\n"
"     ind.y = index.y + i;\n"
"     vec4 L = imageLoad(inBloom0, ivec2(ind));\n"
"     col += L.xyz * gaussianFilter[i];\n"
"     ind.x = index.x;\n"
"     ind.y = index.y - i;\n"
"     L = imageLoad(inBloom0, ivec2(ind));\n"
"     col += L.xyz * gaussianFilter[i];\n"
"   }\n"

"   vec4 Out = vec4(0.0f, 0.0f, 0.0f, 0.0f);\n"
"   Out.xyz = col;\n"
"   imageStore(outBloom1, ivec2(index), Out);\n"
"}\n";

char* ShaderBloom3 =

"layout(binding = 1, set = 0) uniform sampler2D inBloom1[];\n"
"layout(binding = 2, set = 0, rgba8) uniform image2D Output;\n"

"void getImageUV(in uvec2 dtid, out vec2 uv)\n"
"{\n"
"   ivec2 size = imageSize(Output);\n"
"   uv.x = float(dtid.x) / float(size.x);\n"
"   uv.y = float(dtid.y) / float(size.y);\n"
"}\n"

//����
"void main()\n"
"{\n"
"	uvec2 index = gl_GlobalInvocationID.xy;\n"//image�̃T�C�Y�ł̈ʒu
"   vec2 uv;\n"
"   getImageUV(index, uv);\n"

"   vec4 Out = imageLoad(Output, ivec2(index));\n"

"   int numGaus = int(bloomParam.numGaussFilter);\n"

"   vec4 gau = vec4(0.0f, 0.0f, 0.0f, 0.0f);\n"

"   for(int i = 0; i < numGaus; i++){\n"
"      vec4 t = texture(inBloom1[i], uv);\n"
"      gau += t;\n"
"   }\n"

"   gau = gau / bloomParam.numGaussFilter * bloomParam.bloomStrength;\n"
"   imageStore(Output, ivec2(index), gau);\n"
"}\n";

char* ShaderBloom4 =

"#version 460\n"

"layout(local_size_x = 1, local_size_y = 1) in;\n"

"layout(binding = 0, set = 0, rgba8) uniform image2D image;\n"
"layout(binding = 1, set = 0, rgba8) uniform image2D inOutput;\n"

"void main()\n"
"{\n"
"	uvec2 index = gl_GlobalInvocationID.xy;\n"//image�̃T�C�Y�ł̈ʒu

"   vec4 Image = imageLoad(image, ivec2(index));\n"
"   vec4 Bloom = imageLoad(inOutput, ivec2(index));\n"
"   imageStore(inOutput, ivec2(index), Image + Bloom);\n"
"}\n";