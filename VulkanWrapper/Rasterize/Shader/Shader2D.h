//*****************************************************************************************//
//**                                                                                     **//
//**                               Shader2D.h                                            **//
//**                                                                                     **//
//*****************************************************************************************//

char* vsShader2D =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"

"layout (binding = 0) uniform bufferMat {\n"
"    vec2 world;\n"
"} gBufferMat;\n"

"layout(location = 0) in vec2 inPos;\n"
"layout(location = 1) in vec4 color;\n"

"layout(location = 0) out vec4 color_out;\n"
"out gl_PerVertex{ vec4 gl_Position; };\n"

"void main()\n"
"{\n"
"   vec2 pos = inPos;\n"
"   pos.x += gBufferMat.world.x;\n"
"   pos.y += gBufferMat.world.y;\n"
"	gl_Position = vec4(pos, 0.0f, 1.0f);\n"
"	color_out = color;\n"
"}\n";

char* fsShader2D =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"

"layout(location = 0) in vec4 color;\n"
"layout(location = 0) out vec4 color_out;\n"
"void main() { color_out = color; }\n";

char* vsShader2DTex =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"

"layout (binding = 0) uniform bufferMat {\n"
"    vec2 world;\n"
"} gBufferMat;\n"

"layout(location = 0) in vec2 inPos;\n"
"layout(location = 1) in vec2 inTexCoord;\n"

"layout(location = 0) out vec2 outTexCoord;\n"

"void main()\n"
"{\n"
"   vec2 pos = inPos;\n"
"   pos.x += gBufferMat.world.x;\n"
"   pos.y += gBufferMat.world.y;\n"
"	gl_Position = vec4(pos, 0.0f, 1.0f);\n"
"	outTexCoord = inTexCoord;\n"
"}\n";

char* fsShader2DTex =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"

"layout (binding = 1) uniform sampler2D texSampler;\n"

"layout(location = 0) in vec2 inTexCoord;\n"
"layout(location = 0) out vec4 color_out;\n"
"void main() { color_out = texture(texSampler, inTexCoord); }\n";

