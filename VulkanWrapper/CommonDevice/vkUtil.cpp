//*****************************************************************************************//
//**                                                                                     **//
//**                              vkUtil.cpp                                             **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "vkUtil.h"

namespace {
    CoordTf::VECTOR3 subtraction3(CoordTf::VECTOR3 v0, CoordTf::VECTOR3 v1) {
        return { v0.x - v1.x,v0.y - v1.y, v0.z - v1.z };
    }

    CoordTf::VECTOR2 subtraction2(CoordTf::VECTOR2 v0, CoordTf::VECTOR2 v1) {
        return { v0.x - v1.x,v0.y - v1.y };
    }

    CoordTf::VECTOR3 CalcTangent(CoordTf::VECTOR3 v0, CoordTf::VECTOR3 v1, CoordTf::VECTOR3 v2,
        CoordTf::VECTOR2 uv0, CoordTf::VECTOR2 uv1, CoordTf::VECTOR2 uv2, CoordTf::VECTOR3 normal) {

        using namespace CoordTf;

        VECTOR3 deltaPos1 = subtraction3(v1, v0);
        VECTOR3 deltaPos2 = subtraction3(v2, v0);

        VECTOR2 deltaUV1 = subtraction2(uv1, uv0);
        VECTOR2 deltaUV2 = subtraction2(uv2, uv0);

        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

        VectorMultiply(&deltaPos1, deltaUV2.y);
        VectorMultiply(&deltaPos2, deltaUV1.y);
        VECTOR3 tangent = subtraction3(deltaPos1, deltaPos2);

        VectorMultiply(&deltaPos2, deltaUV1.x);
        VectorMultiply(&deltaPos1, deltaUV2.x);
        VECTOR3 bitangent = subtraction3(deltaPos2, deltaPos1);

        VECTOR3 out = {};
        VectorCross(&out, &normal, &tangent);

        if (VectorDot(&out, &bitangent) < 0.0f) {
            tangent.x *= -1.0f;
            tangent.y *= -1.0f;
            tangent.z *= -1.0f;
        }

        return tangent;
    }
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
    int posBytePos, int norBytePos, int texBytePos, int tangentBytePos) {

    unsigned char* b_posSt = (unsigned char*)vertexArr + posBytePos;
    unsigned char* b_norSt = (unsigned char*)vertexArr + norBytePos;
    unsigned char* b_texSt = (unsigned char*)vertexArr + texBytePos;
    unsigned char* b_tanSt = (unsigned char*)vertexArr + tangentBytePos;
    for (int i = 0; i < numMaterial; i++) {
        unsigned int cnt = 0;
        while (indexCntArr[i] > cnt) {
            CoordTf::VECTOR3* posVec[3] = {};
            CoordTf::VECTOR3* norVec[3] = {};
            CoordTf::VECTOR2* texVec[3] = {};
            CoordTf::VECTOR3* tanVec[3] = {};

            for (int ind = 0; ind < 3; ind++) {
                unsigned int index = indexArr[i][cnt++] * structByteStride;
                unsigned char* b_pos = b_posSt + index;
                unsigned char* b_nor = b_norSt + index;
                unsigned char* b_tex = b_texSt + index;
                unsigned char* b_tan = b_tanSt + index;
                posVec[ind] = (CoordTf::VECTOR3*)b_pos;
                norVec[ind] = (CoordTf::VECTOR3*)b_nor;
                texVec[ind] = (CoordTf::VECTOR2*)b_tex;
                tanVec[ind] = (CoordTf::VECTOR3*)b_tan;
            }
            CoordTf::VECTOR3 tangent = CalcTangent(*(posVec[0]), *(posVec[1]), *(posVec[2]),
                *(texVec[0]), *(texVec[1]), *(texVec[2]), *(norVec[0]));

            *(tanVec[0]) = *(tanVec[1]) = *(tanVec[2]) = tangent;
        }
    }
}