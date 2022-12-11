//*****************************************************************************************//
//**                             Shader_raygenDepthMapTest.h                             **//
//*****************************************************************************************//

char* Shader_raygenDepthMapTest =

"void main()\n"
"{\n"
"    raygen_In();\n"

"    vec4 map = imageLoad(depthMap, ivec2(gl_LaunchIDEXT.xy));\n"
"    vec4 dp = vec4(map.x, map.x, map.x, 1.0f);\n"
"    imageStore(image, ivec2(gl_LaunchIDEXT.xy), dp);\n"
"}\n";

