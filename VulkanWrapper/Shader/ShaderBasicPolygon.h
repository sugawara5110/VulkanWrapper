//*****************************************************************************************//
//**                                                                                     **//
//**                            ShaderBasicPolygon                                       **//
//**                                                                                     **//
//*****************************************************************************************//

char* vsShaderBasicPolygon =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"

"layout (std140, binding = 0) uniform bufferVals {\n"
"    mat4 mvp;\n"
"} myBufferVals;\n"

"layout (location = 0) in vec4 pos;\n"
"layout (location = 1) in vec4 inColor;\n"
"layout (location = 2) in vec2 inTexCoord;\n"

"layout (location = 0) out vec4 outColor;\n"
"layout (location = 1) out vec2 outTexCoord;\n"

"void main() {\n"
"   outColor = inColor;\n"
"   outTexCoord = inTexCoord;\n"
"   gl_Position = myBufferVals.mvp * pos;\n"
"}\n";

char* fsShaderBasicPolygon =
"#version 450\n"
"##extension GL_ARB_separate_shader_objects : enable\n"

"layout (binding = 1) uniform sampler2D texSampler;"

"layout (location = 0) in vec4 inColor;\n"
"layout (location = 1) in vec2 inTexCoord;"

"layout (location = 0) out vec4 outColor;\n"

"void main() {\n"
"   outColor = texture(texSampler, inTexCoord);\n"
//"   outColor = vec4(inTexCoord, 0.0, 1.0);"
"}\n";
