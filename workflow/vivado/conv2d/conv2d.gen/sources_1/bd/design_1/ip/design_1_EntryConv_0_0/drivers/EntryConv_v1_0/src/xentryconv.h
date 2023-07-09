// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2023.1 (64-bit)
// Tool Version Limit: 2023.05
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XENTRYCONV_H
#define XENTRYCONV_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xentryconv_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
    u16 DeviceId;
    u64 Control_BaseAddress;
} XEntryconv_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XEntryconv;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XEntryconv_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XEntryconv_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XEntryconv_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XEntryconv_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XEntryconv_Initialize(XEntryconv *InstancePtr, u16 DeviceId);
XEntryconv_Config* XEntryconv_LookupConfig(u16 DeviceId);
int XEntryconv_CfgInitialize(XEntryconv *InstancePtr, XEntryconv_Config *ConfigPtr);
#else
int XEntryconv_Initialize(XEntryconv *InstancePtr, const char* InstanceName);
int XEntryconv_Release(XEntryconv *InstancePtr);
#endif

void XEntryconv_Start(XEntryconv *InstancePtr);
u32 XEntryconv_IsDone(XEntryconv *InstancePtr);
u32 XEntryconv_IsIdle(XEntryconv *InstancePtr);
u32 XEntryconv_IsReady(XEntryconv *InstancePtr);
void XEntryconv_EnableAutoRestart(XEntryconv *InstancePtr);
void XEntryconv_DisableAutoRestart(XEntryconv *InstancePtr);

void XEntryconv_Set_X(XEntryconv *InstancePtr, u64 Data);
u64 XEntryconv_Get_X(XEntryconv *InstancePtr);
void XEntryconv_Set_W(XEntryconv *InstancePtr, u64 Data);
u64 XEntryconv_Get_W(XEntryconv *InstancePtr);
void XEntryconv_Set_Z(XEntryconv *InstancePtr, u64 Data);
u64 XEntryconv_Get_Z(XEntryconv *InstancePtr);

void XEntryconv_InterruptGlobalEnable(XEntryconv *InstancePtr);
void XEntryconv_InterruptGlobalDisable(XEntryconv *InstancePtr);
void XEntryconv_InterruptEnable(XEntryconv *InstancePtr, u32 Mask);
void XEntryconv_InterruptDisable(XEntryconv *InstancePtr, u32 Mask);
void XEntryconv_InterruptClear(XEntryconv *InstancePtr, u32 Mask);
u32 XEntryconv_InterruptGetEnabled(XEntryconv *InstancePtr);
u32 XEntryconv_InterruptGetStatus(XEntryconv *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
