﻿//*****************************************************************************************//
//**                             Shader_hitCom.h                                         **//
//*****************************************************************************************//

char* Shader_hitCom =

"#extension GL_EXT_buffer_reference : enable \n"
"#extension GL_EXT_scalar_block_layout : enable \n"

"hitAttributeEXT vec2 attribs;\n"

"struct Vertex {\n"
"    vec4 Position;\n"
"    vec4 Normal;\n"
"    vec4 tangent;\n"
"    vec4 TexCoord;\n"
"    vec4 SpeTexCoord;\n"
"};\n"

"struct Vertex3 {\n"
"    Vertex pos[3];\n"
"};\n"

"layout(binding = 0, set = 1) uniform sampler2D texturesDif[];\n"
"layout(binding = 0, set = 2) uniform sampler2D texturesNor[];\n"
"layout(binding = 0, set = 3) uniform sampler2D texturesSpe[];\n"

"layout(buffer_reference, scalar) buffer VertexBuffer {\n"
"    Vertex v[];\n"
"};\n"
"layout(buffer_reference, scalar) buffer IndexBuffer {\n"
"    int i[];\n"
"};\n"
"layout(shaderRecordEXT, std430) buffer SBTData {\n"
"    IndexBuffer  indices;\n"//cpp側でのアドレス書き込み順に揃える
"    VertexBuffer verts;\n"//cpp側でのアドレス書き込み順に揃える
"};\n"

///////////////////////////////////////////Material識別////////////////////////////////////////////
"bool materialIdent(uint matNo, uint MaterialBit)\n"
"{\n"
"    return (matNo & MaterialBit) == MaterialBit;\n"
"}\n"

///////////////////////////////////////ヒット位置取得/////////////////////////////////////////////
"vec3 HitWorldPosition()\n"
"{\n"
//               原点           現在のヒットまでの距離      方向
"    return gl_WorldRayOriginEXT + gl_RayTmaxEXT * gl_WorldRayDirectionEXT;\n"
"}\n"

///////////////////////////////////////////hitポリゴン(3頂点)取得//////////////////////////////////
"Vertex3 getVertex3()\n"
"{\n"
//gl_PrimitiveID:ヒットした三角形のインデックス, BLASのprimitiveCountで設定した値
"    const int ind0 = indices.i[gl_PrimitiveID * 3];\n"
"    const int ind1 = indices.i[gl_PrimitiveID * 3 + 1];\n"
"    const int ind2 = indices.i[gl_PrimitiveID * 3 + 2];\n"
"    Vertex3 ret;\n"
"    ret.pos[0] = verts.v[ind0];\n"
"    ret.pos[1] = verts.v[ind1];\n"
"    ret.pos[2] = verts.v[ind2];\n"
"    return ret;\n"
"}\n"

///////////////////////////////////////////頂点座標取得////////////////////////////////////////////
"vec3 getVertex()\n"
"{\n"
"    Vertex3 v3 = getVertex3();\n"

//hitした三角形の重心を計算
"    const vec3 bary = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);\n"

"    vec3 ret;\n"
"    ret = v3.pos[0].Position.xyz * bary.x + v3.pos[1].Position.xyz * bary.y + v3.pos[2].Position.xyz * bary.z;\n"

"    return ret;\n"
"}\n"

///////////////////////////////////////深度値取得//////////////////////////////////////////////////
"float getDepth()\n"
"{\n"
"    vec3 v = getVertex();\n"

"    vec4 ver = vec4(v, 1.0f);\n"

"    MaterialCB mcb = matCB[gl_InstanceID];\n"

"    mat4 m = mcb.mvp;\n"
"    vec4 ver2 = ver * m;\n"
"    return ver2.z / ver2.w;\n"
"}\n"

///////////////////////////////////////////法線取得///////////////////////////////////////////////
"vec3 getNormal()\n"
"{\n"
"    Vertex3 v3 = getVertex3();\n"

"    const vec3 bary = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);\n"

"    vec3 ret;\n"
"    ret = normalize(v3.pos[0].Normal.xyz * bary.x + v3.pos[1].Normal.xyz * bary.y + v3.pos[2].Normal.xyz * bary.z);\n"

"    return ret;\n"
"}\n"

/////////////////////////////////////////接ベクトル取得/////////////////////////////////////////////
"vec3 getTangent()\n"
"{\n"
"    Vertex3 v3 = getVertex3();\n"

"    const vec3 bary = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);\n"

"    vec3 ret;\n"
"    ret = normalize(v3.pos[0].tangent.xyz * bary.x + v3.pos[1].tangent.xyz * bary.y + v3.pos[2].tangent.xyz * bary.z);\n"

"    return ret;\n"
"}\n"

///////////////////////////////////////////TexUV取得///////////////////////////////////////////////
"vec2 getTexUV()\n"
"{\n"
"    Vertex3 v3 = getVertex3();\n"

"    const vec3 bary = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);\n"

"    vec2 ret;\n"
"    ret = v3.pos[0].TexCoord.xy * bary.x + v3.pos[1].TexCoord.xy * bary.y + v3.pos[2].TexCoord.xy * bary.z;\n"

"    return ret;\n"
"}\n"

///////////////////////////////////////////SpeUV取得///////////////////////////////////////////////
"vec2 getSpeUV()\n"
"{\n"
"    Vertex3 v3 = getVertex3();\n"

"    const vec3 bary = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);\n"

"    vec2 ret;\n"
"    ret = v3.pos[0].SpeTexCoord.xy * bary.x + v3.pos[1].SpeTexCoord.xy * bary.y + v3.pos[2].SpeTexCoord.xy * bary.z;\n"

"    return ret;\n"
"}\n"

/////////////////////////////ノーマルテクスチャから法線取得/////////////////////////////////////
"vec3 getNormalMap(in vec3 normal, in vec2 uv, in vec3 tangent)\n"
"{\n"
"    NormalTangent tan;\n"
//接ベクトル計算
"    tan = GetTangent(normal, mat3(gl_ObjectToWorld3x4EXT), tangent);\n"
//法線テクスチャ
"    vec4 Tnor = texture(texturesNor[gl_InstanceID], uv);\n"
//ノーマルマップでの法線出力
"    return GetNormal(Tnor.xyz, tan.normal, tan.tangent);\n"
"}\n"

//////////////////////////////////////ピクセル値取得///////////////////////////////////////////
//////////////ディフェーズ
"vec4 getDifPixel()\n"
"{\n"
"    vec2 uv = getTexUV();\n"
"    vec4 tex = texture(texturesDif[gl_InstanceID], uv);\n"
"    MaterialCB mcb = matCB[gl_InstanceID];\n"
"    vec4 add = mcb.addColor;\n"

"    tex.x = clamp(tex.x + add.x, 0.0f, 1.0f);\n"
"    tex.y = clamp(tex.y + add.y, 0.0f, 1.0f);\n"
"    tex.z = clamp(tex.z + add.z, 0.0f, 1.0f);\n"
"    tex.w = clamp(tex.w + add.w, 0.0f, 1.0f);\n"

"    return tex;\n"
"}\n"
//////////////ノーマル
"vec3 getNorPixel()\n"
"{\n"
"    vec2 uv = getTexUV();\n"
"    vec3 tan = getTangent();\n"
"    vec3 nor = getNormal();\n"

"    vec3 ret = getNormalMap(nor, uv, tan);\n"
"    return ret;\n"
"}\n"
//////////////スペキュラ
"vec3 getSpePixel()\n"
"{\n"
"    vec2 uv = getSpeUV();\n"
"    vec4 ret = texture(texturesSpe[gl_InstanceID], uv);\n"
"    return ret.xyz;\n"
"}\n";