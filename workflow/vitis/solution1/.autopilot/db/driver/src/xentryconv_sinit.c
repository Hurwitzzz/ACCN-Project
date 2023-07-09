// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2023.1 (64-bit)
// Tool Version Limit: 2023.05
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xentryconv.h"

extern XEntryconv_Config XEntryconv_ConfigTable[];

XEntryconv_Config *XEntryconv_LookupConfig(u16 DeviceId) {
	XEntryconv_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XENTRYCONV_NUM_INSTANCES; Index++) {
		if (XEntryconv_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XEntryconv_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XEntryconv_Initialize(XEntryconv *InstancePtr, u16 DeviceId) {
	XEntryconv_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XEntryconv_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XEntryconv_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif

