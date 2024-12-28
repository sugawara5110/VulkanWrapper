//*****************************************************************************************//
//**                             Shader_raygen.h                                         **//
//*****************************************************************************************//

char* Shader_raygen =

"void main()\n"
"{\n"
"    raygen_In();\n"

"    vec3 col = payload.color;\n"
"    vec3 colsatu;\n"
"    colsatu.x = clamp(col.x, 0.0f, 1.0f);\n"
"    colsatu.y = clamp(col.y, 0.0f, 1.0f);\n"
"    colsatu.z = clamp(col.z, 0.0f, 1.0f);\n"

"    const vec2 index = gl_LaunchIDEXT.xy;\n"//この関数を実行しているピクセル座標を取得
"    const vec2 dim = gl_LaunchSizeEXT.xy;\n"//画面全体の幅と高さを取得
"    vec4 Image = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"

"    if (sceneParams.traceMode != 0)\n"
"    {\n"
"       vec4 prev_screenPos4 = vec4(payload.hitPosition, 1.0f) * sceneParams.prevViewProjection;\n"
"       vec2 prev_screenPos = (prev_screenPos4.xy / prev_screenPos4.z * payload.depth);\n"
"       prev_screenPos.y *= -1.0f;\n"
"       uvec2 prevInd = uvec2((prev_screenPos + 1.0f) * dim * 0.5f);\n"
"       float prevDepth = imageLoad(prevDepthMap, ivec2(prevInd)).x;\n"
"       vec3 prevNor = imageLoad(prevNormalMap, ivec2(prevInd)).xyz;\n"
"       float crruentDepth = imageLoad(depthMap, ivec2(index)).x;\n"
"       vec3 crruentNor = imageLoad(normalMap, ivec2(index)).xyz;\n"

"       float frameReset = sceneParams.frameReset_DepthRange_NorRange.x;\n"
"       float DepthRange = sceneParams.frameReset_DepthRange_NorRange.y;\n"
"       float NorRange = sceneParams.frameReset_DepthRange_NorRange.z;\n"

"       if(abs(prevDepth - crruentDepth) <= DepthRange && dot(prevNor, crruentNor) >= NorRange)\n"
"       {\n"
"          float fi = imageLoad(frameIndexMap, ivec2(index)).x;\n"
"          vec4 fmap = vec4(fi + 1.0f, 0.0f, 0.0f, 0.0f);\n"//x:のみ使用
"          imageStore(frameIndexMap, ivec2(index), fmap);\n"
"       }\n"
"       else\n"
"       {\n"
"          vec4 fmap = vec4(0.0f, 0.0f, 0.0f, 0.0f);\n"//x:のみ使用
"          imageStore(frameIndexMap, ivec2(index), fmap);\n"
"       }\n"

"       if(frameReset == 1.0f)\n"
"       {\n"
"          vec4 fmap = vec4(0.0f, 0.0f, 0.0f, 0.0f);\n"//x:のみ使用
"          imageStore(frameIndexMap, ivec2(index), fmap);\n"
"       }\n"

"       float fi = imageLoad(frameIndexMap, ivec2(index)).x;\n"
"       const float CMA_Ratio = 1.0f / (fi + 1.0f);\n"
"       vec3 prev = imageLoad(image, ivec2(index)).xyz;\n"
"       vec3 le = mix(prev, colsatu, CMA_Ratio);\n"
"       Image = vec4(le, 1.0f);\n"
"    }\n"
"    else\n"
"    {\n"
"       Image = vec4(colsatu, 1.0f);\n"
"    }\n"

"    imageStore(image, ivec2(index), Image);\n"
"}\n";

