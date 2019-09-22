//*****************************************************************************************//
//**                                                                                     **//
//**                            ShaderBasicPolygon                                       **//
//**                                                                                     **//
//*****************************************************************************************//

char* vsShaderBasicPolygon =
"#version 450\n"
"#extension GL_ARB_separate_shader_objects : enable\n"

"layout (binding = 0) uniform bufferMat {\n"
"    mat4 world;\n"
"    mat4 mvp;\n"
"} gBufferMat;\n"

"layout (location = 0) in vec4 inPos;\n"
"layout (location = 1) in vec3 inNormal;\n"
"layout (location = 2) in vec2 inTexCoord;\n"

"layout (location = 0) out vec3 outWpos;\n"
"layout (location = 1) out vec3 outNormal;\n"
"layout (location = 2) out vec2 outTexCoord;\n"

"void main() {\n"
"   outWpos = (gBufferMat.world * inPos).xyz;\n"
"   outNormal = normalize(mat3(gBufferMat.world) * inNormal);\n"
"   outTexCoord = inTexCoord;\n"
"   gl_Position = gBufferMat.mvp * inPos;\n"
"}\n";

char* fsShaderBasicPolygon =
"#version 450\n"
"##extension GL_ARB_separate_shader_objects : enable\n"

"layout (binding = 1) uniform sampler2D texSampler;\n"
"layout (binding = 2) uniform bufferMaterial {\n"
"    vec4 diffuse;\n"
"    vec4 specular;\n"
"    vec4 ambient;\n"
"    vec4 viewPos;\n"
"    vec4 lightPos[256];\n"
"    vec4 lightColor[256];\n"
"    vec4 numLight;\n"//ライト数, 減衰1, 減衰2, 減衰3
"} gMaterial;\n"

"layout (location = 0) in vec3 inWpos;\n"
"layout (location = 1) in vec3 inNormal;\n"
"layout (location = 2) in vec2 inTexCoord;"

"layout (location = 0) out vec4 outColor;\n"

"void main() {\n"

"   vec4 col = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
"   for(int i = 0; i < gMaterial.numLight.x; i++)\n"
"   {\n"
"      float distance = length(inWpos - gMaterial.lightPos[i].xyz);\n"
"      float attenuation = 1.0f / \n"
"                          (gMaterial.numLight.y + \n"
"                           gMaterial.numLight.z * distance + \n"
"                           pow(gMaterial.numLight.w * distance, 2.0f));\n"
"      vec3 lightVec = normalize(gMaterial.lightPos[i].xyz - inWpos);\n"
"      float lightScalar = dot(inNormal,lightVec);\n"
"      float ScaCla = clamp(lightScalar,0.0f,1.0f);\n"
"      col.xyz += ScaCla * gMaterial.diffuse.xyz * gMaterial.lightColor[i].xyz * attenuation;\n"
"   }\n"
"   vec4 tex = texture(texSampler, inTexCoord);\n"
"   outColor = tex * col + gMaterial.ambient;\n"
"}\n";
