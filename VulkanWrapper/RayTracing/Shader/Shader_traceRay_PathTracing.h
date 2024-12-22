///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Shader_traceRay_PathTracing.h                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////////////

char* Shader_traceRay_PathTracing = 

///////////////////////G項/////////////////////////////////////////////////////////////////////
"float G(in vec3 hitPosition, in vec3 normal, in RayPayload payload)\n"
"{\n"
"    vec3 lightVec = payload.hitPosition - hitPosition;\n"
"    vec3 light_normal = payload.normal;\n"
"    vec3 hitnormal = normal;\n"
"    vec3 Lvec = normalize(lightVec);\n"
"    float cosine1 = saturate(dot(-Lvec, light_normal));\n"
"    float cosine2 = saturate(dot(Lvec, hitnormal));\n"
"    float distance = length(lightVec);\n"
"    float distAtten = distance * distance;\n"
"    return cosine1 * cosine2 / distAtten;\n"
"}\n"

///////////////////////NeeGetLight///////////////////////////////////////////////////////////////
"RayPayload NeeGetLight(in uint RecursionCnt, in vec3 hitPosition, in vec3 normal, inout int emIndex)\n"
"{\n"
"    uint NumEmissive = numEmissive.x;\n"
/////光源サイズ合計
"    float sumSize = 0.0f;\n"
"    for (uint i = 0; i < NumEmissive; i++)\n"
"    {\n"
"        sumSize += emissiveNo[i].y;\n"
"    }\n"
"    if (useImageBasedLighting)\n"
"        sumSize += IBL_size;\n"

/////乱数を生成
"    uint rnd = Rand_integer() % 101;\n"

/////光源毎のサイズから全光源の割合を計算,そこからインデックスを選択
"    uint sum_min = 0;\n"
"    uint sum_max = 0;\n"
"    for (uint i = 0; i < NumEmissive; i++)\n"
"    {\n"
"        sum_min = sum_max;\n"
"        sum_max += (uint) (emissiveNo[i].y / sumSize * 100.0f);\n"//サイズの割合を累積
"        if (sum_min <= rnd && rnd < sum_max)\n"
"        {\n"
"            emIndex = i;\n"
"            break;\n"
"        }\n"//乱数が累積値の範囲に入ったらそのインデックス値を選択
"    }\n"

"    vec3 ePos;\n"
"    uint ray_flag;\n"

"    if (emIndex >= 0)\n"
"    {\n"
"        ePos = emissivePosition[emIndex].xyz;\n"
"        ray_flag = RAY_FLAG_CULL_FRONT_FACING_TRIANGLES;\n"
"    }\n"
"    else\n"
"    {\n"
"        ePos = hitPosition;\n"
"        ray_flag = RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;\n"
"    }\n"

"    RayDesc ray;\n"
"    ray.Direction = RandomVector(vec3(1.0f, 0.0f, 0.0f), 2.0f);\n"//2.0f全方向

"    RayPayload payload;\n"
"    payload.hitPosition = ePos;\n"
"    payload.mNo = NEE;\n"//処理分岐用

/////光源から点をランダムで取得
"    traceRay(RecursionCnt, ray_flag, 0, 0, ray, payload);\n"

"    if (payload.hit)\n"
"    {\n"
"        vec3 lightVec = payload.hitPosition - hitPosition;\n"
"        ray.Direction = normalize(lightVec);\n"
"        payload.hitPosition = hitPosition;\n"
"        payload.mNo = NEE; //処理分岐用\n"
////////今の位置から取得した光源位置へ飛ばす
"        traceRay(RecursionCnt, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0, 0, ray, payload);\n"
"    }\n"
"    return payload;\n"
"}\n"

///////////////////////NextEventEstimation////////////////////////////////////////////////////
"vec3 NextEventEstimation(in vec3 outDir, in uint RecursionCnt, in vec3 hitPosition, \n"
"                           in float4 difTexColor, in vec3 speTexColor, in vec3 normal,\n"
"                           in bool bsdf_f, in float in_eta, in float out_eta)\n"
"{\n"
"    float norDir = dot(outDir, normal);\n"
"    if (norDir < 0.0f)\n"
"    {\n"//法線が反対側の場合, 物質内部と判断
"        normal *= -1.0f;\n"
"    }\n"
    
"    int emIndex = -1;\n"
"    RayPayload neeP = NeeGetLight(RecursionCnt, hitPosition, normal, emIndex);\n"

"    float g = G(hitPosition, normal, neeP);\n"

"    vec3 inDir = normalize(neeP.hitPosition - hitPosition);\n"

"    vec3 local_inDir = worldToLocal(normal, inDir);\n"
"    vec3 local_outDir = worldToLocal(normal, outDir);\n"

"    float pdf;\n"
"    vec3 bsdf = BSDF(bsdf_f, local_inDir, local_outDir, difTexColor, speTexColor, local_normal, in_eta, out_eta, pdf);\n"

"    float PDF;\n"
"    if (emIndex >= 0)\n"
"    {\n"
"        PDF = LightPDF(emIndex);\n"
"    }\n"
"    else\n"
"    {\n"
"        PDF = IBL_PDF();\n"
"        g = 1.0f;\n"
"    }\n"
    
"    if (getMaterialCB().roughness <= 0.0f)\n"
"    {\n"
"        PDF = 1.0f;\n"
"    }\n"
    
"    return saturate(bsdf * g) * neeP.color / PDF;\n"
"}\n"

///////////////////////PathTracing////////////////////////////////////////////////////////////
"RayPayload PathTracing(in vec3 outDir, in uint RecursionCnt, in vec3 hitPosition, \n"
"                       in float4 difTexColor, in vec3 speTexColor, in vec3 normal, \n"
"                       in vec3 throughput, in uint matNo,\n"
"                       out bool bsdf_f, out float in_eta, out float out_eta)\n"
"{\n"
"    RayPayload payload;\n"
"    payload.hitPosition = hitPosition;\n"

"    float rouPDF = min(max(max(throughput.x, throughput.y), throughput.z), 1.0f);\n"
/////確率的に処理を打ち切り これやらないと白っぽくなる
"    uint rnd = Rand_integer() % 101;\n"
"    if (rnd > (uint) (rouPDF * 100.0f))\n"
"    {\n"
"        payload.throughput = vec3(0.0f, 0.0f, 0.0f);\n"
"        payload.color = vec3(0.0f, 0.0f, 0.0f);\n"
"        payload.hit = false;\n"
"        return payload;\n"
"    }\n"

"    MaterialCB mcb = getMaterialCB();\n"
"    vec3 Diffuse = mcb.Diffuse.xyz;\n"
"    vec3 Speculer = mcb.Speculer.xyz;\n"
"    uint mNo = mcb.materialNo;\n"
"    float roughness = mcb.roughness;\n"

"    float sum_diff = Diffuse.x + Diffuse.y + Diffuse.z;\n"
"    float sum_spe = Speculer.x + Speculer.y + Speculer.z;\n"
"    float sum = sum_diff + sum_spe;\n"
"    uint diff_threshold = (uint) (sum_diff / sum * 100.0f);\n"

"    float Alpha = difTexColor.w;\n"

"    vec3 rDir = vec3(0.0f, 0.0f, 0.0f);\n"

"    in_eta = AIR_RefractiveIndex;\n"
"    out_eta = mcb.RefractiveIndex;\n"

"    float norDir = dot(outDir, normal);\n"
"    if (norDir < 0.0f)\n"
"    {\n"//法線が反対側の場合, 物質内部と判断
"        normal *= -1.0f;\n"
"        in_eta = mcb.RefractiveIndex;\n"
"        out_eta = AIR_RefractiveIndex;\n"
"    }\n"

"    bsdf_f = true;\n"
"    rnd = Rand_integer() % 101;\n"
"    if ((uint) (Alpha * 100.0f) < rnd && materialIdent(mNo, TRANSLUCENCE))\n"
"    {\n"//透過

//////////////eta = 入射前物質の屈折率 / 入射後物質の屈折率
"        float eta = in_eta / out_eta;\n"

"        vec3 eyeVec = -outDir;\n"
"        vec3 refractVec = refract(eyeVec, normal, eta);\n"
"        float Area = roughness * roughness;\n"
        
"        if (roughness <= 0.0f)\n"
"        {\n"
"            rDir = refractVec;\n"
"        }\n"
"        else\n"
"        {\n"
"            rDir = RandomVector(refractVec, Area);\n"
"        }\n"
"    }\n"
"    else\n"
"    {\n"
"        bsdf_f = false;\n"
"        rnd = Rand_integer() % 101;\n"
"        if (diff_threshold < rnd && materialIdent(mNo, METALLIC))\n"
"        {\n"//Speculer
"            vec3 eyeVec = -outDir;\n"
"            vec3 reflectVec = reflect(eyeVec, normal);\n"
"            float Area = roughness * roughness;\n"
"            if (roughness <= 0.0f)\n"
"            {\n"
"                rDir = reflectVec;\n"
"            }\n"
"            else\n"
"            {\n"
"                rDir = RandomVector(reflectVec, Area);\n"
"            }\n"
"        }\n"
"        else\n"
"        {\n"//Diffuse
"            rDir = RandomVector(normal, 1.0f);\n"//1.0f半球
"        }\n"
"    }\n"

"    RayDesc ray;\n"
"    ray.Direction = rDir;\n"

"    vec3 local_inDir = worldToLocal(normal, ray.Direction);\n"
"    vec3 local_outDir = worldToLocal(normal, outDir);\n"

"    float PDF = 0.0f;\n"
"    float cosine = abs(dot(local_normal, local_inDir));\n"

"    vec3 bsdf = BSDF(bsdf_f, local_inDir, local_outDir, difTexColor, speTexColor, local_normal, in_eta, out_eta, PDF);\n"

"    throughput *= (bsdf * cosine / PDF / rouPDF);\n"

"    payload.throughput = throughput;\n"

"    payload.hitPosition = hitPosition;\n"
"    payload.mNo = matNo;\n"//処理分岐用

"    traceRay(RecursionCnt, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0, 0, ray, payload);\n"//透過マテリアルはcpp側でフラグ処理済み

"    payload.throughput = throughput;\n"

/////NEE実行時かつ『NONREFLECTION』はパストレでの光源衝突は寄与しない

"    if (payload.hit && matNo == NEE_PATHTRACER && materialIdent(mNo, NONREFLECTION))\n"
"    {\n"
"        payload.color = vec3(0.0f, 0.0f, 0.0f);\n"
"    }\n"

"    return payload;\n"
"}\n"

///////////////////////PayloadCalculate_PathTracing///////////////////////////////////////////
"vec3 PayloadCalculate_PathTracing(in uint RecursionCnt, in vec3 hitPosition, \n"
"                                    in float4 difTexColor, in vec3 speTexColor, in vec3 normal, \n"
"                                    in vec3 throughput, inout int hitInstanceId)\n"
"{\n"
"    vec3 ret = difTexColor.xyz;\n"

"    hitInstanceId = (int) getInstancingID();\n"

"    MaterialCB mcb = getMaterialCB();\n"
"    uint mNo = mcb.materialNo;\n"

"    vec3 outDir = -WorldRayDirection();\n"

/////PathTracing
"    uint matNo = NEE_PATHTRACER;\n"
"    if (traceMode == 1)\n"
"        matNo = EMISSIVE;\n"

"    bool bsdf_f;\n"
"    float in_eta;\n"
"    float out_eta;\n"
"    RayPayload pathPay = PathTracing(outDir, RecursionCnt, hitPosition, difTexColor, speTexColor, normal, throughput, matNo,\n"
"                                     bsdf_f, in_eta, out_eta);\n"

/////NextEventEstimation
"    if (traceMode == 2)\n"
"    {\n"
"        vec3 neeCol = NextEventEstimation(outDir, RecursionCnt, hitPosition, difTexColor, speTexColor, normal,\n"
"                                            bsdf_f, in_eta, out_eta);\n"

"        if (pathPay.hit && !materialIdent(mNo, NONREFLECTION))\n"
"        {\n"
"            ret = pathPay.color * pathPay.throughput;\n"//光源ヒットかつ反射透過の場合, 通常パストレと同じ寄与
"        }\n"
"        else\n"
"        {\n"
"            ret = pathPay.color + neeCol * pathPay.throughput;\n"//通常NEE処理
"        }\n"
"    }\n"
"    else\n"
"    {\n"
"        if (pathPay.hit)\n"
"        {\n"
"            ret = pathPay.color * pathPay.throughput;\n"//光源ヒット時のみthroughputを乗算し値を返す
"        }\n"
"        else\n"
"        {\n"
"            ret = pathPay.color;\n"//光源ヒットがない場合はそのまま値を返す
"        }\n"
"    }\n"
"    hitInstanceId = pathPay.hitInstanceId;\n"

"    return ret;\n"
"}\n";
