//*****************************************************************************************//
//**                                                                                     **//
//**                              vkUtil.h                                               **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef vkUtil_Header
#define vkUtil_Header

#ifdef __ANDROID__
 #include <android/log.h>
 #include <android/native_window.h>
 #define NEW new
#else
 #define VK_USE_PLATFORM_WIN32_KHR

 #if defined(DEBUG) || defined(_DEBUG)
  #define _CRTDBG_MAP_ALLOC
  #include <stdlib.h>
  #include <crtdbg.h>
  #define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
 #else
  #define NEW new
 #endif

 #include <windows.h>
#endif

#include "NVIDIA_Library/extensions_vk.hpp"
#include "VulkanPFN.h"
#include "../../CoordTf/CoordTf.h"

#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include <tuple>
#include <functional>
#include <shaderc/shaderc.hpp>

#ifdef __ANDROID__
#else
#pragma comment(lib, "vulkan-1")
#pragma comment(lib, "shaderc_shared.lib")
#endif

namespace vkUtil {
    template<typename TYPE>
    void S_DELETE(TYPE& p) { if (p) { delete p;    p = nullptr; } }
    template<typename TYPE>
    void ARR_DELETE(TYPE& p) { if (p) { delete[] p;    p = nullptr; } }
    void checkError(VkResult res);

    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT, uint64_t object,
        size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);

    void calculationMatrixWorld(CoordTf::MATRIX& World, CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale);

    class addChar {
    public:
        char* str = nullptr;
        size_t size = 0;

        void addStr(char* str1, char* str2);

        ~addChar() {
            ARR_DELETE(str);
        }
    };

    void createTangent(int numMaterial, unsigned int* indexCntArr,
        void* vertexArr, unsigned int** indexArr, int structByteStride,
        int posBytePos, int norBytePos, int texBytePos, int tangentBytePos);

    CoordTf::VECTOR3 normalRecalculation(CoordTf::VECTOR3 Nor[3]);

    char* getNameFromPass(char* pass);

    template<class T>
    T Align(T size, uint32_t align) {
        return (size + align - 1) & ~static_cast<T>((align - 1));
    }

    void memory_leak_test();
}

#endif