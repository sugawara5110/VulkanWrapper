//*****************************************************************************************//
//**                             Shader_anyHit.h                                         **//
//*****************************************************************************************//

char* Shader_anyHit =

"void main()\n"
"{\n"
//�e�N�X�`���擾
"    vec4 difTex = getDifPixel();\n"
"    float Alpha = difTex.w;\n"

"    if (Alpha <= 0.0f)\n"
"    {\n"
"        ignoreIntersectionEXT;\n"
"    }\n"
"}\n";


