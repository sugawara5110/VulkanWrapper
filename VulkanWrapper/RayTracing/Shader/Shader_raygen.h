//*****************************************************************************************//
//**                             Shader_raygen.h                                         **//
//*****************************************************************************************//

char* Shader_raygen =

"void main()\n"
"{\n"
"    raygen_In();\n"

"    vec3 col = payload.color;\n"
"    vec3 colsatu;\n"
"    colsatu.x = clamp(col.x, 0.0f, 1.0f);\n"
"    colsatu.y = clamp(col.y, 0.0f, 1.0f);\n"
"    colsatu.z = clamp(col.z, 0.0f, 1.0f);\n"

"    imageStore(image, ivec2(gl_LaunchIDEXT.xy), vec4(colsatu, 1.0f));\n"
"}\n";

