//*****************************************************************************************//
//**                             Shader_raygenInstanceIdMapTest.h                        **//
//*****************************************************************************************//

char* Shader_raygenInstanceIdMapTest =

"void main()\n"
"{\n"
"    raygen_In();\n"

"    vec4 map = imageLoad(InstanceIdMap, ivec2(gl_LaunchIDEXT.xy));\n"
"    int id = int(map.x);\n"
"    int numInstance = int(sceneParams.maxRecursion.y);\n"

"    vec3 ret;\n"
"    if(id == -1)ret = vec3(0,0,0);\n"

"    int step = 4096 / numInstance;\n"
"    int color3 = 0;\n"

"    for(int i = 0; i < numInstance; i++){\n"
"       int r = color3 >> 8;\n"
"       int g = (color3 >> 4) & 0x00f;\n"
"       int b = color3 & 0x00f;\n"
"       float rf = float(r) / 15.0f;\n"
"       float gf = float(g) / 15.0f;\n"
"       float bf = float(b) / 15.0f;\n"
"       if(id == i){\n"
"         ret = vec3(rf, gf, bf);\n"
"         break;\n"
"       }\n"
"       color3 += step;\n"
"    }\n"

"    imageStore(image, ivec2(gl_LaunchIDEXT.xy), vec4(ret, 1.0f));\n"
"}\n";

