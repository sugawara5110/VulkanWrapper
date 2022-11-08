///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                  ShaderNormalTangent                                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////////////

char* ShaderNormalTangent =

"struct NormalTangent\n"
"{\n"
"    vec3 normal;\n"
"    vec3 tangent;\n"
"};\n"

"NormalTangent GetTangent(vec3 normal, mat3 world, vec3 tangent)\n"
"{\n"
"	NormalTangent Out;\n"

"   Out.normal = normal * world;\n"
"   Out.normal = normalize(Out.normal);\n"
"   Out.tangent = tangent * world;\n"
"   Out.tangent = normalize(Out.tangent);\n"

"   return Out;\n"
"}\n"

"vec3 normalTexConvert(vec3 norT, vec3 normal, vec3 tangent)\n"
"{\n"
"   vec3 N = normal;\n"
"   vec3 T = normalize(tangent - dot(tangent, N) * N);\n"
"   vec3 B = cross(N, T);\n"
"   mat3 TBN = mat3(T, B, N);\n"

"   return norT * TBN;\n"
"}\n"

"vec3 GetNormal(vec3 norTex, vec3 normal, vec3 tangent)\n"
"{\n"
"   vec3 norT = norTex * 2.0f - 1.0f;\n"
"   return normalTexConvert(norT, normal, tangent);\n"
"}\n";
