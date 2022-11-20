//*****************************************************************************************//
//**                             Shader_raygen.h                                         **//
//*****************************************************************************************//

char* Shader_raygen =

"layout(location = 0) rayPayloadEXT vkRayPayload payload;\n"
"layout(binding = 1, set = 0, rgba8) uniform image2D image;\n"

"void main()\n"
"{\n"
"    const vec2 index = gl_LaunchIDEXT.xy;\n"//この関数を実行しているピクセル座標を取得
"    const vec2 dim = gl_LaunchSizeEXT.xy;\n"//画面全体の幅と高さを取得
"    const vec2 screenPos = (index + 0.5f) / dim * 2.0f - 1.0f;\n"

"    vec3 origin = sceneParams.cameraPosition.xyz;\n"
"    vec4 world = vec4(screenPos.x, -screenPos.y, 0, 1) * sceneParams.projectionToWorld;\n"
"    world.xyz /= world.w;\n"
"    vec3 direction = normalize(world.xyz - origin);\n"

"    float tmin = sceneParams.TMin_TMax.x;\n"
"    float tmax = sceneParams.TMin_TMax.y;\n"

"    payload.RecursionCnt = 1;\n"
"    bool loop = true;\n"
"    payload.hitPosition = origin;\n"
//"    gDepthOut[index] = 1.0f;\n"  後で追加
//"    payload.depth = -1.0f;\n"
"    payload.reTry = false;\n"

"    while(loop){\n"

"       origin = payload.hitPosition;\n"

"       traceRayEXT(\n"
"           topLevelAS,\n"
"           gl_RayFlagsCullBackFacingTrianglesEXT,\n"
"           0xff,\n"
"           0,\n"//sbtRecordOffset
"           0,\n"//sbtRecordStride
"           0,\n"//missIndex
"           origin.xyz,\n"
"           tmin,\n"
"           direction.xyz,\n"
"           tmax,\n"
"           0\n"//ペイロードインデックス
"       );\n"

"       loop = payload.reTry;\n"

"    }\n"

//"    if(payload.depth != -1.0f)"
//"       gDepthOut[index] = payload.depth;\n"
"    vec3 col = payload.color;\n"
"    vec3 colsatu;\n"
"    colsatu.x = clamp(col.x, 0.0f, 1.0f);\n"
"    colsatu.y = clamp(col.y, 0.0f, 1.0f);\n"
"    colsatu.z = clamp(col.z, 0.0f, 1.0f);\n"

"    imageStore(image, ivec2(gl_LaunchIDEXT.xy), vec4(colsatu, 1.0));\n"
"}\n";

