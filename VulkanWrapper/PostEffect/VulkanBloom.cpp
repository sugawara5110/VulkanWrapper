//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanBloom.cpp                                        **//
//**                                                                                     **//
//*****************************************************************************************//

#include "VulkanBloom.h"

namespace {

	float* gaussian(float sigma, int& numWid) {//sd:•ªŽU ”’l‚ª‘å‚«‚¢’ö‘S‘Ì“I‚É“¯“™‚É‚È‚é

		int wCnt = 0;
		while (expf(-(float)(wCnt * wCnt) / (sigma * sigma * 2.0f)) > 0.0001f) {
			if (wCnt++ > 1000)break;
		}

		numWid = wCnt;
		float total = 0.0f;
		float* gaArr = new float[numWid];

		for (int x = 0; x < numWid; x++)
		{
			total +=
				gaArr[x] = expf(-(float)(x * x) / (sigma * sigma * 2.0f));
		}

		for (int i = 0; i < numWid; i++)
		{
			gaArr[i] /= total;
		}

		return gaArr;
	}
}

void VulkanBloom::setImage(
	VulkanDevice::ImageSet* raytracedImage, VulkanDevice::ImageSet* instanceIdMap,
	uint32_t width, uint32_t height,
	uint32_t* emissiveInstanceId, uint32_t numEmissive) {

	pRaytracedImage = raytracedImage;
	pInstanceIdMap = instanceIdMap;
	Width = width;
	Height = height;
	NumEmissive = numEmissive;
	memcpy(EmissiveInstanceId, emissiveInstanceId, sizeof(uint32_t) * NumEmissive);
}