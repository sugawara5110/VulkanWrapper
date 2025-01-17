//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanBasicPolygonRt.cpp                                 **//
//**                                                                                     **//
//*****************************************************************************************//

#include "VulkanBasicPolygonRt.h"

void VulkanBasicPolygonRt::RtData::setvSize(CoordTf::VECTOR3 v) {
    if (!setvSize_first) {
        LmaxX = LminX = v.x;
        LmaxY = LminY = v.y;
        LmaxZ = LminZ = v.z;
        setvSize_first = true;
    }
    if (LmaxX < v.x)LmaxX = v.x;
    if (LminX > v.x)LminX = v.x;
    if (LmaxY < v.y)LmaxY = v.y;
    if (LminY > v.y)LminY = v.y;
    if (LmaxZ < v.z)LmaxZ = v.z;
    if (LminZ > v.z)LminZ = v.z;
}

void VulkanBasicPolygonRt::RtData::createOutlineSize(CoordTf::VECTOR3 scale, int InstanceIndex) {
    float x = (LmaxX - LminX) * scale.x;
    float y = (LmaxY - LminY) * scale.y;
    float z = (LmaxZ - LminZ) * scale.z;

    instance[InstanceIndex].OutlineSize = 2 * (x * y + y * z + x * z);
}

VulkanBasicPolygonRt::VulkanBasicPolygonRt() {

}

VulkanBasicPolygonRt::~VulkanBasicPolygonRt() {
    for (uint32_t i = 0; i < numSwap; i++) {
        vertexBuf[i].destroy();
    }
    for (auto i = 0; i < Rdata.size(); i++) {
        Rdata[i].indexBuf.destroy();
        for (uint32_t i1 = 0; i1 < numSwap; i1++) {
            Rdata[i].BLAS[i1].destroy();
        }
        Rdata[i].texId.destroy();
    }
}

void VulkanBasicPolygonRt::setMaterialType(vkMaterialType type, uint32_t matIndex) {
    Rdata[matIndex].mat.MaterialType.x = (float)type;
}

void VulkanBasicPolygonRt::LightOn(bool on, uint32_t InstanceIndex, uint32_t matIndex,
    float range, float att1, float att2, float att3) {

    Rdata[matIndex].instance[InstanceIndex].lightst.as(range, att1, att2, att3);
    Rdata[matIndex].instance[InstanceIndex].lightOn = 0.0f;
    if (on)Rdata[matIndex].instance[InstanceIndex].lightOn = 1.0f;
}

void VulkanBasicPolygonRt::createVertexBuffer(
    uint32_t QueueIndex,
    uint32_t comIndex,
    Vertex3D_t* ver, uint32_t num) {

    using namespace CoordTf;

    struct Vertex3Dvec4 {
        VECTOR4 pos = {};
        VECTOR4 normal = {};
        VECTOR4 tangent = {};
        VECTOR4 difUv = {};
        VECTOR4 speUv = {};
    };

    Vertex3Dvec4* v = NEW Vertex3Dvec4[num];

    for (uint32_t i = 0; i < num; i++) {
        memcpy(&v[i].pos, &ver[i].pos, sizeof(VECTOR3));
        memcpy(&v[i].normal, &ver[i].normal, sizeof(VECTOR3));
        memcpy(&v[i].tangent, &ver[i].tangent, sizeof(VECTOR3));
        memcpy(&v[i].difUv, &ver[i].difUv, sizeof(VECTOR2));
        memcpy(&v[i].speUv, &ver[i].speUv, sizeof(VECTOR2));
        for (auto k = 0; k < Rdata.size(); k++) {
            Rdata[k].setvSize(ver[i].pos);
        }
    }

    VkBufferUsageFlags usageForRT =
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{
  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, nullptr,
    };
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    void* pNext = &memoryAllocateFlagsInfo;

    for (uint32_t i = 0; i < numSwap; i++) {
        vertexBuf[i].createVertexBuffer(QueueIndex, comIndex, v, num, false, pNext, &usageForRT);
    }
    vkUtil::ARR_DELETE(v);
    vertexStride = sizeof(Vertex3Dvec4);
    vertexCount = num;
}

void VulkanBasicPolygonRt::createIndexBuffer(
    RtData& rdata,
    uint32_t QueueIndex,
    uint32_t comIndex,
    uint32_t* ind, uint32_t indNum) {

    VkBufferUsageFlags usageForRT =
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{
  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, nullptr,
    };
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    void* pNext = &memoryAllocateFlagsInfo;

    rdata.indexBuf.createVertexBuffer(QueueIndex, comIndex, ind, indNum, true, pNext, &usageForRT);
    rdata.indexCount = indNum;
}

void VulkanBasicPolygonRt::createBLAS(RtData& rdata, uint32_t QueueIndex, uint32_t comIndex) {
    VkAccelerationStructureGeometryKHR Geometry{
           VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR
    };

    Geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    if (useAlpha) {
        Geometry.flags = 0;
    }

    Geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;

    auto& triangles = Geometry.geometry.triangles;

    for (uint32_t i = 0; i < numSwap; i++) {
        triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        triangles.vertexData.deviceAddress = rdata.vertexBuf[i]->getDeviceAddress();
        triangles.maxVertex = rdata.vertexCount;
        triangles.vertexStride = rdata.vertexStride;
        triangles.indexType = VK_INDEX_TYPE_UINT32;
        triangles.indexData.deviceAddress = rdata.indexBuf.getDeviceAddress();

        VkAccelerationStructureBuildRangeInfoKHR BuildRangeInfo{};
        BuildRangeInfo.primitiveCount = rdata.indexCount / 3;
        BuildRangeInfo.primitiveOffset = 0;
        BuildRangeInfo.firstVertex = 0;
        BuildRangeInfo.transformOffset = 0;

        VkBuildAccelerationStructureFlagsKHR buildFlags = 0;
        buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        if (UpdateBLAS_On) {
            buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
        }

        rdata.BLAS[i].buildAS(QueueIndex, comIndex, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
            Geometry,
            BuildRangeInfo,
            buildFlags);
        rdata.BLAS[i].destroyScratchBuffer();
    }
}

void VulkanBasicPolygonRt::updateBLAS(uint32_t swapIndex, RtData& rdata, uint32_t QueueIndex, uint32_t comIndex) {

    VkBuildAccelerationStructureFlagsKHR buildFlags = 0;
    buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    rdata.BLAS[swapIndex].update(
        QueueIndex,
        comIndex,
        VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        buildFlags);
}

void VulkanBasicPolygonRt::updateInstance(uint32_t swapIndex, RtData& rdata) {

    for (uint32_t i = 0; i < rdata.instance.size(); i++) {
        VkAccelerationStructureInstanceKHR instance{};
        instance.instanceCustomIndex = 0;
        instance.mask = 0x00;
        if (InstanceCnt > i) instance.mask = 0xFF;
        instance.flags = 0;
        instance.accelerationStructureReference = rdata.BLAS[swapIndex].getDeviceAddress();
        instance.instanceShaderBindingTableRecordOffset = rdata.hitShaderIndex;
        if (rdataCreateF)
            instance.transform = rdata.instance[i].vkWorld;
        else
            instance.transform = VkTransformMatrixKHR();
        rdata.instance[i].vkInstance[swapIndex] = instance;
    }
}

void VulkanBasicPolygonRt::createTexture(RtData& rdata, uint32_t QueueIndex, uint32_t comIndex, VulkanDevice::textureIdSetInput& tId) {
    rdata.texId.diffuseId = tId.diffuseId;
    rdata.texId.normalId = tId.normalId;
    rdata.texId.specularId = tId.specularId;

    VulkanDevice* d = VulkanDevice::GetInstance();
    d->createTextureSet(QueueIndex, comIndex, rdata.texId);
}

void VulkanBasicPolygonRt::createMultipleMaterials(uint32_t QueueIndex, uint32_t comIndex, bool use_Alpha, uint32_t numMat,
    Vertex3D_t* ver, uint32_t num, uint32_t** ind, uint32_t* indNum,
    VulkanDevice::textureIdSetInput* texid, uint32_t numInstance, bool updateBLAS_on) {

    UpdateBLAS_On = updateBLAS_on;

    vkUtil::createTangent(numMat, indNum,
        ver, ind, sizeof(Vertex3D_t), 0, 3 * 4, 9 * 4, 6 * 4);

    Rdata.resize(numMat);
    createVertexBuffer(QueueIndex, comIndex, ver, num);
    for (auto i = 0; i < Rdata.size(); i++) {
        Rdata[i].instance.resize((size_t)numInstance);
        for (uint32_t i1 = 0; i1 < numSwap; i1++) {
            Rdata[i].vertexBuf[i1] = &vertexBuf[i1];
        }
        Rdata[i].vertexCount = vertexCount;
        Rdata[i].vertexStride = vertexStride;
        createIndexBuffer(Rdata[i], QueueIndex, comIndex, ind[i], indNum[i]);
        createBLAS(Rdata[i], QueueIndex, comIndex);
        for (uint32_t i1 = 0; i1 < numSwap; i1++) {
            updateInstance(i1, Rdata[i]);
        }
        createTexture(Rdata[i], QueueIndex, comIndex, texid[i]);
        useAlpha = use_Alpha;
    }
    rdataCreateF = true;
}

void VulkanBasicPolygonRt::create(uint32_t QueueIndex, uint32_t comIndex, bool useAlpha,
    VulkanDevice::Vertex3D* ver, uint32_t num, uint32_t* ind, uint32_t indNum,
    int32_t difTexInd, int32_t norTexInd, int32_t speTexInd, uint32_t numInstance) {

    Vertex3D_t* v3 = NEW Vertex3D_t[num];

    using namespace CoordTf;

    for (uint32_t i = 0; i < num; i++) {
        memcpy(&v3[i].pos, &ver[i].pos, sizeof(VECTOR3));
        memcpy(&v3[i].normal, &ver[i].normal, sizeof(VECTOR3));
        memcpy(&v3[i].difUv, &ver[i].difUv, sizeof(VECTOR2));
        memcpy(&v3[i].speUv, &ver[i].speUv, sizeof(VECTOR2));
    }

    const uint32_t numMaterial = 1;
    VulkanDevice::textureIdSetInput tex[numMaterial];
    tex[0].diffuseId = difTexInd;
    tex[0].normalId = norTexInd;
    tex[0].specularId = speTexInd;

    createMultipleMaterials(QueueIndex, comIndex, useAlpha, 1,
        v3, num, &ind, &indNum,
        tex, numInstance, false);

    vkUtil::ARR_DELETE(v3);
}

void VulkanBasicPolygonRt::setMaterialColor(
    CoordTf::VECTOR3 diffuse, CoordTf::VECTOR3 specular, CoordTf::VECTOR3 ambient,
    uint32_t materialIndex) {

    VulkanBasicPolygonRt::RtMaterial& m = Rdata[materialIndex].mat;
    m.vDiffuse.as(diffuse.x, diffuse.y, diffuse.z, 1.0f);
    m.vSpeculer.as(specular.x, specular.y, specular.z, 1.0f);
    m.vAmbient.as(ambient.x, ambient.y, ambient.z, 1.0f);
}

void VulkanBasicPolygonRt::setMaterialShininess(float shininess, uint32_t materialIndex) {

    VulkanBasicPolygonRt::RtMaterial& m = Rdata[materialIndex].mat;
    m.shininess.x = shininess;
}

void VulkanBasicPolygonRt::setMaterialRefractiveIndex(float RefractiveIndex, uint32_t materialIndex) {

    VulkanBasicPolygonRt::RtMaterial& m = Rdata[materialIndex].mat;
    m.RefractiveIndex_roughness.x = RefractiveIndex;
}

void VulkanBasicPolygonRt::setMaterialRoughness(float Roughness, uint32_t materialIndex) {
    VulkanBasicPolygonRt::RtMaterial& m = Rdata[materialIndex].mat;
    m.RefractiveIndex_roughness.y = Roughness;
}

void VulkanBasicPolygonRt::instancing(CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale, CoordTf::VECTOR4 addColor) {
    using namespace CoordTf;
    MATRIX w = {};
    MATRIX w2 = {};
    VkTransformMatrixKHR vw = {};
    vkUtil::calculationMatrixWorld(w, pos, theta, scale);
    memcpy(&w2, &w, sizeof(MATRIX));
    MatrixTranspose(&w);
    memcpy(&vw.matrix[0], &w.m[0], sizeof(float) * 4);
    memcpy(&vw.matrix[1], &w.m[1], sizeof(float) * 4);
    memcpy(&vw.matrix[2], &w.m[2], sizeof(float) * 4);

    VulkanDevice* device = VulkanDevice::GetInstance();

    for (auto i = 0; i < Rdata.size(); i++) {
        Rdata[i].createOutlineSize(scale, InstanceCnt);
        VulkanBasicPolygonRt::Instance& ins = Rdata[i].instance[InstanceCnt];
        ins.addColor = addColor;
        ins.world = w2;
        ins.vkWorld = vw;
        ins.mvp = w2 * device->getCameraView() * device->getProjection();
        MatrixTranspose(&ins.mvp);
        if (InstanceCnt > Rdata[i].instance.size()) {
            throw std::runtime_error("InstanceCnt exceeded rdata.instance.size! Check the number of executions of instancing()");
        }
    }
    InstanceCnt++;
}

void VulkanBasicPolygonRt::instancingUpdate(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex) {
    for (auto i = 0; i < Rdata.size(); i++) {
        if (UpdateBLAS_On) {
            updateBLAS(swapIndex, Rdata[i], QueueIndex, comIndex);
        }
        updateInstance(swapIndex, Rdata[i]);
    }
    InstanceCnt = 0;
}

void VulkanBasicPolygonRt::update(uint32_t swapIndex, uint32_t QueueIndex,
    uint32_t comIndex, CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale, CoordTf::VECTOR4 addColor) {
    instancing(pos, theta, scale, addColor);
    instancingUpdate(swapIndex, QueueIndex, comIndex);
}
