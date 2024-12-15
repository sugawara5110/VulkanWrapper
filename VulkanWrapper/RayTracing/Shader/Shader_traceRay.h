//*****************************************************************************************//
//**                             Shader_traceRay                                         **//
//*****************************************************************************************//

char* Shader_traceRay =

"layout(location = 0) rayPayloadEXT vkRayPayload payload;\n"

"void traceRay(in int RecursionCnt,\n"
"              in uint RayFlags,\n"
"              in uint HitGroupIndex,\n"
"              in uint MissShaderIndex,\n"
"              in vec3 direction)\n"
"{\n"
"    float tmin = sceneParams.TMin_TMax.x;\n"
"    float tmax = sceneParams.TMin_TMax.y;\n"
"    payload.color = vec3(0.0f, 0.0f, 0.0f);\n"
"    payload.RecursionCnt = RecursionCnt + 1;\n"
"    payload.EmissiveIndex = 0;\n"
"    payload.reTry = false;\n"
"    payload.hit = false;\n"

"    if (RecursionCnt <= sceneParams.maxRecursion.x)\n"
"    {\n"
"        bool loop = true;\n"
"        while (loop)\n"
"        {\n"
"            vec3 origin = payload.hitPosition;\n"
"            traceRayEXT(topLevelAS, RayFlags, 0xff, HitGroupIndex, 0, MissShaderIndex, origin, tmin, direction, tmax, 0);\n"
"            loop = payload.reTry;\n"
"        }\n"
"    }\n"
"}\n";