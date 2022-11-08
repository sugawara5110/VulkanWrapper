//*****************************************************************************************//
//**                                                                                     **//
//**                             Shader_raygen.h                                         **//
//**                                                                                     **//
//*****************************************************************************************//

char* Shader_raygen =

"layout(location = 0) rayPayloadEXT vkRayPayload payload;\n"

"layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;\n"
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

"    float tmin = 0.00;\n"
"    float tmax = 10000.0;\n"

"    traceRayEXT(\n"
"        topLevelAS,\n"
"        gl_RayFlagsOpaqueEXT,\n"
"        0xff,\n"
"        0,\n"
"        0,\n"
"        0,\n"
"        origin.xyz,\n"
"        tmin,\n"
"        direction.xyz,\n"
"        tmax,\n"
"        0\n"
"    );\n"
"    imageStore(image, ivec2(gl_LaunchIDEXT.xy), vec4(payload.color, 1.0));\n"
"}\n";

