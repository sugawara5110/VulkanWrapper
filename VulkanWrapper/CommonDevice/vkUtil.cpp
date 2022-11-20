//*****************************************************************************************//
//**                                                                                     **//
//**                              vkUtil.cpp                                             **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "vkUtil.h"

static CoordTf::VECTOR3 CalcTangent(CoordTf::VECTOR3 normal, CoordTf::VECTOR3 upVec) {
    using namespace CoordTf;

    //upVec‚ÍŒÅ’è‚Æ‚·‚é

    const int arrNum = 6;

    const VECTOR3 upNor[arrNum] = {
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f}
    };
    const VECTOR3 upTan[arrNum] = {
        {1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, 1.0f}
    };

    VECTOR3 tangent = {};
    VectorCross(&tangent, &normal, &upVec);

    for (int i = 0; i < arrNum; i++) {
        if (normal.x == upNor[i].x &&
            normal.y == upNor[i].y &&
            normal.z == upNor[i].z) {

            tangent = upTan[i];
            break;
        }
    }

    VECTOR3 ret = {};
    VectorNormalize(&ret, &tangent);
    return ret;
}

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
    MATRIX rotZ, rotY, rotX, rotZY, rotZYX;
    MATRIX sca;
    MATRIX scro;
    MatrixScaling(&sca, scale.x, scale.y, scale.z);
    MatrixRotationZ(&rotZ, theta.z);
    MatrixRotationY(&rotY, theta.y);
    MatrixRotationX(&rotX, theta.x);
    MatrixMultiply(&rotZY, &rotZ, &rotY);
    MatrixMultiply(&rotZYX, &rotZY, &rotX);
    MatrixTranslation(&mov, pos.x, pos.y, pos.z);
    MatrixMultiply(&scro, &rotZYX, &sca);
    MatrixMultiply(&World, &scro, &mov);
}

void vkUtil::addChar::addStr(char* str1, char* str2) {
    size_t size1 = strlen(str1);
    size_t size2 = strlen(str2);
    size = size1 + size2 + 1;
    str = new char[size];
    memcpy(str, str1, size1 + 1);
    strncat(str, str2, size2 + 1);
}

void vkUtil::createTangent(int numMaterial, unsigned int* indexCntArr,
    void* vertexArr, unsigned int** indexArr, int structByteStride,
    int norBytePos, int tangentBytePos, CoordTf::VECTOR3 upVec) {

    unsigned char* b_norSt = (unsigned char*)vertexArr + norBytePos;
    unsigned char* b_tanSt = (unsigned char*)vertexArr + tangentBytePos;
    for (int i = 0; i < numMaterial; i++) {
        unsigned int cnt = 0;
        while (indexCntArr[i] > cnt) {
            CoordTf::VECTOR3* norVec[3] = {};
            CoordTf::VECTOR3* tanVec[3] = {};

            for (int ind = 0; ind < 3; ind++) {
                unsigned int index = indexArr[i][cnt++] * structByteStride;
                unsigned char* b_nor = b_norSt + index;
                unsigned char* b_tan = b_tanSt + index;
                norVec[ind] = (CoordTf::VECTOR3*)b_nor;
                tanVec[ind] = (CoordTf::VECTOR3*)b_tan;

                *(tanVec[ind]) = CalcTangent(*(norVec[ind]), upVec);
            }
        }
    }
}