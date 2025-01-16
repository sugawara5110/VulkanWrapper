//*****************************************************************************************//
//**                                                                                     **//
//**                              vkUtil.cpp                                             **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "vkUtil.h"

void vkUtil::checkError(VkResult res) {
    if (res != VK_SUCCESS)
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_ERROR, "VulkanERROR", "NOT_VK_SUCCESS");
#else
        throw std::runtime_error(std::to_string(res).c_str());
#endif
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkUtil::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT, uint64_t object,
    size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
#ifdef __ANDROID__
#else
    OutputDebugString(L"Vulkan DebugCall: "); OutputDebugStringA(pMessage); OutputDebugString(L"\n");
#endif
    return VK_FALSE;
}

void vkUtil::calculationMatrixWorld(CoordTf::MATRIX& World, CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale) {
    using namespace CoordTf;
    MATRIX mov;
    MATRIX rotZ, rotY, rotX;
    MATRIX sca;
    MatrixScaling(&sca, scale.x, scale.y, scale.z);
    MatrixRotationZ(&rotZ, theta.z);
    MatrixRotationY(&rotY, theta.y);
    MatrixRotationX(&rotX, theta.x);
    MATRIX rotZYX = rotZ * rotY * rotX;
    MatrixTranslation(&mov, pos.x, pos.y, pos.z);
    World = sca * rotZYX * mov;
}

void vkUtil::addChar::addStr(char* str1, char* str2) {
    size_t size1 = strlen(str1);
    size_t size2 = strlen(str2);
    size = size1 + size2 + 1;
    str = NEW char[size];
    memcpy(str, str1, size1 + 1);
    strncat(str, str2, size2 + 1);
}

void vkUtil::createTangent(int numMaterial, unsigned int* indexCntArr,
    void* vertexArr, unsigned int** indexArr, int structByteStride,
    int posBytePos, int norBytePos, int texBytePos, int tangentBytePos) {

    using namespace CoordTf;

    unsigned char* b_posSt = (unsigned char*)vertexArr + posBytePos;
    unsigned char* b_norSt = (unsigned char*)vertexArr + norBytePos;
    unsigned char* b_texSt = (unsigned char*)vertexArr + texBytePos;
    unsigned char* b_tanSt = (unsigned char*)vertexArr + tangentBytePos;
    for (int i = 0; i < numMaterial; i++) {
        unsigned int cnt = 0;
        while (indexCntArr[i] > cnt) {
            VECTOR3* posVec[3] = {};
            VECTOR3* norVec[3] = {};
            VECTOR2* texVec[3] = {};
            VECTOR3* tanVec[3] = {};

            for (int ind = 0; ind < 3; ind++) {
                unsigned int index = indexArr[i][cnt++] * structByteStride;
                unsigned char* b_pos = b_posSt + index;
                unsigned char* b_nor = b_norSt + index;
                unsigned char* b_tex = b_texSt + index;
                unsigned char* b_tan = b_tanSt + index;
                posVec[ind] = (VECTOR3*)b_pos;
                norVec[ind] = (VECTOR3*)b_nor;
                texVec[ind] = (VECTOR2*)b_tex;
                tanVec[ind] = (VECTOR3*)b_tan;
            }
            VECTOR3 tangent = CalcTangent(*(posVec[0]), *(posVec[1]), *(posVec[2]),
                *(texVec[0]), *(texVec[1]), *(texVec[2]), *(norVec[0]));

            VECTOR3 outN = {};
            VectorNormalize(&outN, &tangent);

            *(tanVec[0]) = *(tanVec[1]) = *(tanVec[2]) = outN;
        }
    }
}

CoordTf::VECTOR3 vkUtil::normalRecalculation(CoordTf::VECTOR3 N[3]) {
    using namespace CoordTf;
    VECTOR3 vecX = {};
    VECTOR3 vecY = {};
    vecX.as(N[0].x - N[1].x, N[0].y - N[1].y, N[0].z - N[1].z);
    vecY.as(N[0].x - N[2].x, N[0].y - N[2].y, N[0].z - N[2].z);
    VECTOR3 vec = {};
    VectorCross(&vec, &vecX, &vecY);
    return vec;
}

char* vkUtil::getNameFromPass(char* pass) {

    uint32_t len = (uint32_t)strlen(pass);
    pass += len;//終端文字を指している

    for (uint32_t i = 0; i < len; i++) {
        pass--;
        if (*pass == '\\' || *pass == '/') {
            pass++;
            break;
        }
    }
    return pass;//ポインタ操作してるので返り値を使用させる
}

void vkUtil::memory_leak_test() {
#ifdef __ANDROID__
#else
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif
}