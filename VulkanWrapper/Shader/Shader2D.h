//*****************************************************************************************//
//**                                                                                     **//
//**                               Shader2D.h                                            **//
//**                                                                                     **//
//*****************************************************************************************//

char* vsShader2D =
"#version 400\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"

"layout(location = 0) in vec2 pos;\n"
"layout(location = 1) in vec4 color;\n"

"layout(location = 0) out vec4 color_out;\n"
"out gl_PerVertex{ vec4 gl_Position; };\n"

"void main()\n"
"{\n"
"	gl_Position = vec4(pos, 0.0f, 1.0f);\n"
"	color_out = color;\n"
"}\n";

char* fsShader2D =
"#version 400\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"layout(location = 0) in vec4 color;\n"
"layout(location = 0) out vec4 color_out;\n"
"void main() { color_out = color; }\n";
