//*****************************************************************************************//
//**                             Shader_raygen_In.h                                      **//
//*****************************************************************************************//

char* Shader_raygen_In =

"layout(binding = 1, set = 0, rgba8) uniform image2D image;\n"
"layout(binding = 3, set = 0, r32f) uniform image2D InstanceIdMap;\n"
"layout(binding = 4, set = 0, r32f) uniform image2D depthMap;\n"

"void raygen_In()\n"
"{\n"
"    const vec2 index = gl_LaunchIDEXT.xy;\n"//この関数を実行しているピクセル座標を取得
"    const vec2 dim = gl_LaunchSizeEXT.xy;\n"//画面全体の幅と高さを取得
"    const vec2 screenPos = (index + 0.5f) / dim * 2.0f - 1.0f;\n"

"    vec3 origin = sceneParams.cameraPosition.xyz;\n"
"    vec4 world = vec4(screenPos.x, -screenPos.y, 0, 1) * sceneParams.projectionToWorld;\n"
"    world.xyz /= world.w;\n"
"    vec3 direction = normalize(world.xyz - origin);\n"

"    payload.hitInstanceId = -1;\n"

"    payload.RecursionCnt = 0;\n"
"    payload.hitPosition = origin;\n"

"    vec4 idMap = vec4(0.0f, 0.0f, 0.0f, 0.0f);\n"//x:のみ使用
"    vec4 dpMap = vec4(0.0f, 0.0f, 0.0f, 0.0f);\n"//x:のみ使用

"    payload.depth = -1.0f;\n"
"    dpMap.x = 1.0f;\n"

"    traceRay(payload.RecursionCnt,\n"
"             gl_RayFlagsCullBackFacingTrianglesEXT,\n"
"             0,\n"
"             0,\n"
"             direction);\n"

"    idMap.x = payload.hitInstanceId;\n"

"    if(payload.depth != -1.0f){\n"
"       dpMap.x = payload.depth;\n"
"    }\n"

"    imageStore(depthMap, ivec2(gl_LaunchIDEXT.xy), dpMap);\n"
"    imageStore(InstanceIdMap, ivec2(gl_LaunchIDEXT.xy), idMap);\n"
"};\n";