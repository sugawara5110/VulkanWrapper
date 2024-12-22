///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Shader_hitCom_PathTracing.h                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////////////

char* Shader_hitCom_PathTracing =

///////////////////////////////////////////ランダムfloat///////////////////////////////////////////
"float Rand_float(vec2 v2)\n"
"{\n"
"	Seed++;\n"
"	return sin(dot(v2, vec2(12.9898, 78.233)) * (sceneParams.SeedFrame % 100 + 1) * 0.001 + Seed + sceneParams.SeedFrame) * 43758.5453;\n"
"}\n"

///////////////////////////////////////////ランダム整数////////////////////////////////////////////
"uint Rand_integer()\n"
"{\n"
"	vec2 index = gl_LaunchIDEXT.xy;\n"
"	return uint(abs(Rand_float(index)));\n"
"}\n"

///////////////////////////////////////////ランダム少数////////////////////////////////////////////
"float Rand_frac(vec2 v2)\n"
"{\n"
"	return fract(Rand_float(v2));\n"
"}\n"

///////////////////////////////////////////ランダムベクトル////////////////////////////////////////
"vec3 RandomVector(vec3 v, float area)\n"
"{\n"
"	vec2 index = gl_LaunchIDEXT.xy;\n"
"	float rand1 = Rand_frac(index);\n"
"	float rand2 = Rand_frac(index + 0.5f);\n"

//ランダムなベクトルを生成
"	float z = area * rand1 - 1.0f;\n"
"	float phi = PI * (2.0f * rand2 - 1.0f);\n"
"	float sq = sqrt(1.0f - z * z);\n"
"	float x = sq * cos(phi);\n"
"	float y = sq * sin(phi);\n"
"	vec3 randV = vec3(x, y, z);\n"

"	return -localToWorld(v, randV);\n"
"}\n"

///////////////////////////////////////////LightPDF////////////////////////////////////////////////
"float LightPDF(uint emIndex)\n"
"{\n"
"	float NumEmissive = float(sceneParams.numEmissive.x);\n"
"	float emSize = sceneParams.emissiveNo[emIndex].y;\n"
"	return 1.0f / (NumEmissive * emSize);\n"
"}\n"

///////////////////////////////////////////IBL_PDF////////////////////////////////////////////////
"float IBL_PDF()\n"
"{\n"
"	return 1.0f / (4 * PI);\n"
"}\n"

///////////////////////////////////////////radiusPDF///////////////////////////////////////////////
"float radiusPDF()\n"
"{\n"
"	return 1.0f / (2 * PI);\n"
"}\n"

///////////////////////////////////////////DiffuseBRDF/////////////////////////////////////////////
"vec3 DiffuseBRDF(vec3 diffuse)\n"
"{\n"
"	return diffuse / PI;\n"
"}\n"

///////////////////////////////////////////CosinePDF///////////////////////////////////////////////
"float CosinePDF(float dotNL)\n"
"{\n"
"	return dotNL / PI;\n"
"}\n"

///////////////////////////////////////////GGX_PDF/////////////////////////////////////////////////
"float GGX_PDF(float NDF, float dotNH, float dotVH)\n"
"{\n"
"	return NDF * dotNH / (4 * dotVH);\n"
"}\n"

///////////////////////////////////////////GGX_GeometrySchlick/////////////////////////////////////
"float GGX_GeometrySchlick(float dotNX, float roughness)\n"
"{\n"
"	float a = roughness * roughness;\n"
"	float k = a / 2.0f;\n"
"	return dotNX / (dotNX * (1 - k) + k);\n"
"}\n"

///////////////////////////////////////////GGX_Distribution////////////////////////////////////////
"float GGX_Distribution(vec3 N, vec3 H, float roughness)\n"
"{\n"
"	float a = roughness * roughness;\n"
"	float a2 = a * a;\n"
"	float dotNH = max(0.0, dot(N, H));\n"
"	float dotNH2 = dotNH * dotNH;\n"

"	float d = (dotNH2 * (a2 - 1.0f) + 1.0f);\n"
"	d *= PI * d;\n"

"	return a2 / d;\n"
"}\n"

///////////////////////////////////////////GGX_GeometrySmith///////////////////////////////////////
"float GGX_GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)\n"
"{\n"
"	float dotNV = abs(dot(N, V));\n"
"	float dotNL = abs(dot(N, L));\n"
"	return GGX_GeometrySchlick(dotNV, roughness) * GGX_GeometrySchlick(dotNL, roughness);\n"
"}\n"

///////////////////////////////////////////FresnelSchlick/////////////////////////////////////////
"vec3 FresnelSchlick(float dotVH, vec3 F0)\n"
"{\n"
"	return F0 + (1 - F0) * pow(1 - dotVH, 5.0);\n"
"}\n"

///////////////////////////////////////////SpecularBRDF///////////////////////////////////////////
"vec3 SpecularBRDF(float D, float G, vec3 F, vec3 V, vec3 L, vec3 N)\n"
"{\n"
"	float dotNL = abs(dot(N, L));\n"
"	float dotNV = abs(dot(N, V));\n"
"	return (D * G * F) / (4 * dotNV * dotNL + 0.001f);\n"
"}\n"
"vec3 FullySpecularBRDF(vec3 F, vec3 L, vec3 N)\n"
"{\n"
"    float dotNL = abs(dot(N, L));\n"
"    return F / dotNL;\n"
"}\n"

///////////////////////////////////////////DiffSpeBSDF/////////////////////////////////////////////
"vec3 DiffSpeBSDF(vec3 inDir, vec3 outDir, vec3 difTexColor, vec3 speTexColor, vec3 normal, out float PDF)\n"
"{\n"
"    const MaterialCB mcb = matCB[gl_InstanceID];\n"
"    const vec3 Diffuse = mcb.Diffuse.xyz;\n"
"    const vec3 Speculer = mcb.Speculer.xyz;\n"
"    const float roughness = mcb.RefractiveIndex_roughness.y;\n"

"    if (dot(normal, inDir) <= 0)\n"
"    {\n"
"        PDF = 1.0f;\n"
"        return vec3(0, 0, 0);\n"
"    }\n"

"    const float sum_diff = Diffuse.x + Diffuse.y + Diffuse.z;\n"
"    const float sum_spe = Speculer.x + Speculer.y + Speculer.z;\n"
"    const float sum = sum_diff + sum_spe;\n"
"    const float diff_threshold = sum_diff / sum;\n"
"    const float spe_threshold = sum_spe / sum;\n"

"    const vec3 H = normalize(inDir + outDir);\n"

"    const float dotNL = abs(dot(normal, inDir));\n"
"    const float dotNH = abs(dot(normal, H));\n"
"    const float dotVH = abs(dot(outDir, H));\n"

"    vec3 F0 = 0.08.xxx;\n"
"    F0 = mix(F0 * speTexColor, difTexColor, (spe_threshold).xxx);\n"

"    const float NDF = GGX_Distribution(normal, H, roughness);\n"
"    const float G = GGX_GeometrySmith(normal, outDir, inDir, roughness);\n"
"    const vec3 F = FresnelSchlick(max(dot(outDir, H), 0), F0);\n"
"    const vec3 kD = (1 - F) * (1 - spe_threshold);\n"

"    vec3 speBRDF;\n"
"    float spePDF;\n"

"    if (roughness <= 0.0f)\n"
"    {\n"
"        speBRDF = FullySpecularBRDF(F, inDir, normal);\n"
"        spePDF = 1.0f;\n"
"    }\n"
"    else\n"
"    {\n"
"        speBRDF = SpecularBRDF(NDF, G, F, outDir, inDir, normal);\n"
"        spePDF = GGX_PDF(NDF, dotNH, dotVH);\n"
"    }\n"

"    const vec3 diffBRDF = DiffuseBRDF(difTexColor);\n"
"    const float diffPDF = CosinePDF(dotNL);\n"
"    const vec3 sumBSDF = (diffBRDF * kD + speBRDF) * dotNL;\n"
"    const float sumPDF = diff_threshold * diffPDF + spe_threshold * spePDF;\n"

"    if (sumPDF <= 0)\n"
"    {\n"
"        PDF = 1.0f;\n"
"        return vec3(0, 0, 0);\n"
"    }\n"

"    PDF = sumPDF;\n"
"    return sumBSDF;\n"
"}\n"

///////////////////////////////////////////RefractionBTDF///////////////////////////////////////////
"vec3 RefractionBTDF(float D, float G, vec3 F, vec3 V, vec3 L, vec3 N, vec3 H, float in_eta, float out_eta)\n"
"{\n"
"    const float dotNL = abs(dot(N, L));\n"
"    const float dotNV = abs(dot(N, V));\n"
"    const float dotHL = abs(dot(H, L));\n"
"    const float dotHV = abs(dot(H, V));\n"

"    const float a = dotHL * dotHV / (dotNL * dotNV);\n"
"    const vec3 b = out_eta * out_eta * (1 - F) * G * D;\n"
"    const float c = pow((in_eta * dotHL + out_eta * dotHV), 2) + 0.001f;\n"
"    return a * b / c;\n"
"}\n"
"vec3 FullyRefractionBTDF(vec3 F, vec3 L, vec3 N, float in_eta, float out_eta)\n"
"{\n"
"    const float dotNL = abs(dot(N, L));\n"

"    float a = out_eta * out_eta / (in_eta * in_eta);\n"
"    vec3 b = 1 - F;\n"
"    float c = 1.0f / dotNL;\n"
"    return a * b * c;\n"
"}\n"

///////////////////////////////////////////RefSpeBSDF//////////////////////////////////////////////
"vec3 RefSpeBSDF(vec3 inDir, vec3 outDir, vec4 difTexColor, vec3 speTexColor, vec3 N, vec3 H, \n"
"                  float in_eta, float out_eta, out float PDF)\n"
"{\n"
"    const float Alpha = difTexColor.w;\n"
"    const float speRatio = Alpha;\n"

"    const float dotNL = abs(dot(N, inDir));\n"
"    const float dotNV = abs(dot(N, outDir));\n"
"    const float dotNH = abs(dot(N, H));\n"
"    const float dotVH = abs(dot(outDir, H));\n"
"    const float dotLH = abs(dot(inDir, H));\n"

"    const MaterialCB mcb = matCB[gl_InstanceID];\n"
"    const vec3 Diffuse = mcb.Diffuse.xyz * difTexColor.xyz;\n"
"    const vec3 Speculer = mcb.Speculer.xyz * speTexColor;\n"
"    const float roughness = mcb.RefractiveIndex_roughness.y;\n"

"    vec3 F0 = 0.08.xxx * Speculer;\n"
"    vec3 F = FresnelSchlick(max(dot(H, outDir), 0), F0);\n"

"    float NDF = GGX_Distribution(N, H, roughness);\n"
"    float G = GGX_GeometrySmith(N, outDir, inDir, roughness);\n"

"    vec3 speBRDF;\n"
"    float spePDF;\n"
"    vec3 refrBTDF;\n"
"    float refrPDF;\n"

"    if (roughness <= 0.0f)\n"
"    {\n"
"        speBRDF = FullySpecularBRDF(F, inDir, N);\n"
"        spePDF = 1.0f;\n"
"        refrBTDF = FullyRefractionBTDF(F, inDir, N, in_eta, out_eta);\n"
"        refrPDF = 1.0f;\n"
"    }\n"
"    else\n"
"    {\n"
"        speBRDF = SpecularBRDF(NDF, G, F, outDir, inDir, N);\n"
"        spePDF = GGX_PDF(NDF, dotNH, dotVH);\n"
"        refrBTDF = RefractionBTDF(NDF, G, F, outDir, inDir, N, H, in_eta, out_eta);\n"
"        refrPDF = GGX_PDF(NDF, dotNH, dotVH);\n"
"    }\n"

"    const vec3 sumBSDF = (speBRDF + refrBTDF * Diffuse) * dotNL;\n"
"    const float sumPDF = speRatio * spePDF + (1 - speRatio) * refrPDF;\n"

"    if (sumPDF <= 0)\n"
"    {\n"
"        PDF = 1.0f;\n"
"        return vec3(0, 0, 0);\n"
"    }\n"

"    PDF = sumPDF;\n"
"    return sumBSDF;\n"
"}\n"

///////////////////////////////////////////BSDF////////////////////////////////////////////////////
"vec3 BSDF(bool bsdf_f, vec3 inDir, vec3 outDir, vec4 difTexColor, vec3 speTexColor, vec3 N,\n"
"            float in_eta, float out_eta, out float PDF)\n"
"{\n"
"    vec3 bsdf;\n"

"    if (bsdf_f)\n"
"    {\n"
"        const vec3 H = normalize(-inDir + outDir);\n"
"        bsdf = RefSpeBSDF(-inDir, outDir, difTexColor, speTexColor, N, H, in_eta, out_eta, PDF);\n"
"    }\n"
"    else\n"
"    {\n"
"        bsdf = DiffSpeBSDF(inDir, outDir, difTexColor.xyz, speTexColor, N, PDF);\n"
"    }\n"
"    return bsdf;\n"
"}\n"

////////////////////////////////////////////SkyLight////////////////////////////////////////////////
"layout(binding = 4, set = 5) uniform sampler2D ImageBasedLighting;\n"

"vec3 getSkyLight(vec3 dir)\n"
"{\n"
"    vec2 uv = vec2(atan(dir.z, dir.x) / 2.0f / PI + 0.5f, acos(dir.y) / PI);\n"
"    vec4 ret = texture(ImageBasedLighting, uv);\n"
"    return ret.xyz;\n"
"}\n";
