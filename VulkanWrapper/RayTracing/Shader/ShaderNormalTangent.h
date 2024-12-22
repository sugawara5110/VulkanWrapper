///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                  ShaderNormalTangent                                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////////////

char* ShaderNormalTangent =

"struct NormalTangent\n"
"{\n"
"   vec3 normal;\n"
"   vec3 tangent;\n"
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

"void getTangentBinormal(in vec3 N, out vec3 T, out vec3 B)\n"
"{\n"
"	vec3 a = abs(N);\n"
"	uint xm = ((a.x - a.y) < 0 && (a.x - a.z) < 0) ? 1 : 0;\n"
"	uint ym = (a.y - a.z) < 0 ? (1 ^ xm) : 0;\n"
"	uint zm = 1 ^ (xm | ym);\n"
"	B = cross(N, vec3(xm, ym, zm));\n"
"	T = cross(B, N);\n"
"}\n"

"vec3 worldToLocal(vec3 N, vec3 localVec)\n"
"{\n"
"	vec3 T;\n"
"	vec3 B;\n"
"	getTangentBinormal(N, T, B);\n"
"	return normalize(vec3(dot(T, localVec), dot(B, localVec), dot(N, localVec)));\n"
"}\n"

"vec3 localToWorld(vec3 N, vec3 localVec)\n"
"{\n"
"	vec3 T;\n"
"	vec3 B;\n"
"	getTangentBinormal(N, T, B);\n"
"	return normalize(T * localVec.x + B * localVec.y + N * localVec.z);\n"
"}\n"

"vec3 normalTexConvert(vec3 norT, vec3 normal, vec3 tangent)\n"
"{\n"
"   vec3 N = normal;\n"
"   vec3 T = tangent;\n"
"   vec3 B = cross(T, N);\n"

"   return normalize(T * norT.x + B * norT.y + N * norT.z);\n"
"}\n"

"vec3 GetNormal(vec3 norTex, vec3 normal, vec3 tangent)\n"
"{\n"
"   vec3 norT = norTex * 2.0f - 1.0f;\n"
"   return normalTexConvert(norT, normal, tangent);\n"
"}\n";
