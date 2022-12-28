///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                           ShaderBloom.h                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////////////

char* ShaderBloom_Com =

"#version 460\n"

"layout(local_size_x = 1, local_size_y = 1) in;\n"

"layout(binding = 0, set = 0) uniform BloomParam {\n"
"    float GaussianWid;\n"//ガウス幅
"    float bloomStrength;\n"//ブルーム強さ
"    float thresholdLuminance;\n"//輝度閾値
"    float numGaussFilter;\n"//ガウスフィルター数
"    int   InstanceID;\n"//処理対象ID
"} bloomParam;\n";

char* ShaderBloom0 =

"layout(binding = 1, set = 0, rgba8) uniform image2D image;\n"//レンダリング完了後の画像
"layout(binding = 2, set = 0, r32f) uniform image2D InstanceIdMap;\n"
"layout(binding = 3, set = 0, rgba8) uniform image2D outLuminance;\n"

"void getImageUV(in uvec2 dtid, out vec2 uv)\n"
"{\n"
"   ivec2 size = imageSize(image);\n"
"   uv.x = float(dtid.x) / float(size.x);\n"
"   uv.y = float(dtid.y) / float(size.y);\n"
"}\n"

"void getLuminanceUV(in uvec2 dtid, out vec2 uv)\n"
"{\n"
"   ivec2 size = imageSize(outLuminance);\n"
"   uv.x = float(dtid.x) / float(size.x);\n"
"   uv.y = float(dtid.y) / float(size.y);\n"
"}\n"

//ブルーム対象のInstanceID毎に輝度を抽出
"void main()\n"
"{\n"
"	uvec2 index = gl_GlobalInvocationID.xy;\n"//outLuminanceのサイズでの位置
"   vec2 uv;\n"
"   getLuminanceUV(index, uv);\n"//outLuminanceのuv

"   ivec2 isize = imageSize(image);\n"//レンダリング完了後の画像サイズ
"   vec2 fsize;\n"
"   fsize.x = float(isize.x) * uv.x;\n"//レンダリング画像サイズのuv計算(InstanceIdMapと同じ)
"   fsize.y = float(isize.y) * uv.y;\n"

"   vec4 L = imageLoad(image, ivec2(fsize));\n"//レンダリング完了後の画像
"   L.w = 0.0f;\n"

"   vec4 map = imageLoad(InstanceIdMap, ivec2(fsize));\n"
"   int instanceId = int(map.x);\n"//マップ上のID読み込み

"   if(L.x + L.y + L.z > bloomParam.thresholdLuminance * 3.0f && \n"//閾値より高いか
"      bloomParam.InstanceID == instanceId)\n"//対象IDと一致するか
"   {\n"
"     imageStore(outLuminance, ivec2(index), L);\n"//一致した場合書き込み,(これでマップ範囲だけ書き込まれる)
"   }else\n"
"   {\n"
"     imageStore(outLuminance, ivec2(index), vec4(0.0f ,0.0f ,0.0f ,0.0f));\n"//一致しない場合はゼロ
"   }\n"
"}\n";

