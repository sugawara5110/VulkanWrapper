///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                           ShaderBloom.h                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////////////

char* ShaderBloom0 =

"#version 460\n"

"layout(local_size_x = 1, local_size_y = 1) in;\n"

"layout(binding = 0, set = 0, rgba8) uniform image2D image;\n"
"layout(binding = 1, set = 0, r32f) uniform image2D InstanceIdMap;\n"
"layout(binding = 2, set = 0) uniform image2D outLuminance;\n"

"layout(binding = 3, set = 0) uniform BloomParam {\n"
"    float GaussianWid;\n"
"    float bloomStrength;\n"
"    float thresholdLuminance;\n"
"    float numGaussFilter;\n"
"    int   InstanceID;\n"
"} bloomParam;\n"

"void main()\n"
"{\n"

"}\n"
;

