//*****************************************************************************************//
//**                                                                                     **//
//**                               Shader2D.h                                            **//
//**                                                                                     **//
//*****************************************************************************************//

char* vsShader2D =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"

"struct Instancing{\n"
"    mat4 world;\n"
"    vec4 pXpYmXmY;\n"
"    vec4 addCol;\n"
"    vec4 d2;\n"
"    vec4 d3;\n"
"};\n"

"layout (binding = 0) uniform bufferMat {\n"
"    Instancing ins[replace_NUM_Ins_CB];\n"
"} gBufferMat;\n"

"layout(location = 0) in vec2 inPos;\n"
"layout(location = 1) in vec4 color;\n"

"layout(location = 0) out vec4 color_out;\n"
"out gl_PerVertex{ vec4 gl_Position; };\n"

"void main()\n"
"{\n"
"   mat4 world = gBufferMat.ins[gl_InstanceIndex].world;\n"
"   vec4 addCol = gBufferMat.ins[gl_InstanceIndex].addCol;\n"
"   vec4 pos = vec4(inPos, 0.0f, 1.0f);\n"
"	gl_Position = world * pos;\n"
"	color_out = color + addCol;\n"
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

"struct Instancing{\n"
"    mat4 world;\n"
"    vec4 pXpYmXmY;\n"
"    vec4 addCol;\n"
"    vec4 d2;\n"
"    vec4 d3;\n"
"};\n"

"layout (binding = 0) uniform bufferMat {\n"
"    Instancing ins[replace_NUM_Ins_CB];\n"
"} gBufferMat;\n"

"layout(location = 0) in vec2 inPos;\n"
"layout(location = 1) in vec2 inTexCoord;\n"

"layout(location = 0) out vec2 outTexCoord;\n"
"layout(location = 1) out vec4 outAddCol;\n"

"void main()\n"
"{\n"
"   mat4 world = gBufferMat.ins[gl_InstanceIndex].world;\n"
"   outAddCol = gBufferMat.ins[gl_InstanceIndex].addCol;\n"
"   vec4 pos = vec4(inPos, 0.0f, 1.0f);\n"
"	gl_Position = world * pos;\n"

"   vec4 pXpYmXmY = gBufferMat.ins[gl_InstanceIndex].pXpYmXmY;\n"

"   outTexCoord.x = inTexCoord.x * pXpYmXmY.x + pXpYmXmY.x * pXpYmXmY.z;\n"
"   outTexCoord.y = inTexCoord.y * pXpYmXmY.y + pXpYmXmY.y * pXpYmXmY.w;\n"
"}\n";

char* fsShader2DTex =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"

"layout (binding = 1) uniform sampler2D texSampler;\n"

"layout(location = 0) in vec2 inTexCoord;\n"
"layout(location = 1) in vec4 inAddCol;\n"

"layout(location = 0) out vec4 color_out;\n"
"void main() { color_out = texture(texSampler, inTexCoord) + inAddCol; }\n";

