//*****************************************************************************************//
//**                                                                                     **//
//**                             Shader_closesthit.h                                     **//
//**                                                                                     **//
//*****************************************************************************************//

char* Shader_closesthit =

"void main()\n"
"{\n"
"    vec3 ver = getVertex();\n"
"    vec3 nor = getNormal();\n"
"    vec2 uv = getTexUV();\n"
"    vec3 worldNormal = mat3(gl_ObjectToWorldEXT) * nor;\n"

"    const vec3 toLightDir = normalize(-sceneParams.dDirection.xyz);\n"
"    const vec3 lightColor = sceneParams.dLightColor.xyz;\n"

"    vec4 dTex = getDifPixel();\n"
"    const vec3 vtxcolor = dTex.xyz;\n"
"    float dotNL = max(dot(worldNormal, toLightDir), 0.0);\n"
"    payload.color = vtxcolor * dotNL * lightColor;\n"
"    payload.color += vtxcolor * sceneParams.GlobalAmbientColor.xyz;\n"
"}\n";
