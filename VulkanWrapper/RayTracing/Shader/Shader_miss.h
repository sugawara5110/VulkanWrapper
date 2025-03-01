//*****************************************************************************************//
//**                             Shader_miss.h                                           **//
//*****************************************************************************************//

char* Shader_miss =

"layout(location = 1) rayPayloadInEXT vkRayPayload payloadIn;\n"

////////////////////////////////////////////SkyLight////////////////////////////////////////////////
"layout(binding = 4, set = 5) uniform sampler2D ImageBasedLighting;\n"

"vec3 getSkyLight(vec3 dir)\n"
"{\n"
"    vec2 uv = vec2(atan(dir.z, dir.x) / 2.0f / PI + 0.5f, acos(dir.y) / PI);\n"
"    vec4 ret = texture(ImageBasedLighting, uv);\n"
"    return ret.xyz;\n"
"}\n"

"void main()\n"
"{\n"
"    payloadIn.color = vec3(0.0, 0.0, 0.0);\n"
"    payloadIn.hit = false;\n"
"    payloadIn.reTry = false;\n"
"    payloadIn.mNo = NONE;\n"

"    if (sceneParams.useImageBasedLighting)\n"
"    {\n"
"       payloadIn.color = getSkyLight(gl_WorldRayDirectionEXT * mat3(sceneParams.ImageBasedLighting_Matrix));\n"
"       payloadIn.hitPosition = gl_WorldRayOriginEXT + gl_RayTmaxEXT * gl_WorldRayDirectionEXT;\n"

"       if (sceneParams.traceMode != 0)\n"
"       {\n"
"          payloadIn.hit = true;\n"
"          payloadIn.mNo = EMISSIVE;\n"
"       }\n"
"    }\n"
"}\n";
