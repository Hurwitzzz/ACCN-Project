// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2023.1 (64-bit)
// Tool Version Limit: 2023.05
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xentryconv.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XEntryconv_CfgInitialize(XEntryconv *InstancePtr, XEntryconv_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XEntryconv_Start(XEntryconv *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XEntryconv_ReadReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_AP_CTRL) & 0x80;
    XEntryconv_WriteReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XEntryconv_IsDone(XEntryconv *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XEntryconv_ReadReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XEntryconv_IsIdle(XEntryconv *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XEntryconv_ReadReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XEntryconv_IsReady(XEntryconv *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XEntryconv_ReadReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XEntryconv_EnableAutoRestart(XEntryconv *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XEntryconv_WriteReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XEntryconv_DisableAutoRestart(XEntryconv *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XEntryconv_WriteReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_AP_CTRL, 0);
}

void XEntryconv_Set_X(XEntryconv *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XEntryconv_WriteReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_X_DATA, (u32)(Data));
    XEntryconv_WriteReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_X_DATA + 4, (u32)(Data >> 32));
}

u64 XEntryconv_Get_X(XEntryconv *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XEntryconv_ReadReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_X_DATA);
    Data += (u64)XEntryconv_ReadReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_X_DATA + 4) << 32;
    return Data;
}

void XEntryconv_Set_W(XEntryconv *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XEntryconv_WriteReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_W_DATA, (u32)(Data));
    XEntryconv_WriteReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_W_DATA + 4, (u32)(Data >> 32));
}

u64 XEntryconv_Get_W(XEntryconv *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XEntryconv_ReadReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_W_DATA);
    Data += (u64)XEntryconv_ReadReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_W_DATA + 4) << 32;
    return Data;
}

void XEntryconv_Set_Z(XEntryconv *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XEntryconv_WriteReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_Z_DATA, (u32)(Data));
    XEntryconv_WriteReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_Z_DATA + 4, (u32)(Data >> 32));
}

u64 XEntryconv_Get_Z(XEntryconv *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XEntryconv_ReadReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_Z_DATA);
    Data += (u64)XEntryconv_ReadReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_Z_DATA + 4) << 32;
    return Data;
}

void XEntryconv_InterruptGlobalEnable(XEntryconv *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XEntryconv_WriteReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_GIE, 1);
}

void XEntryconv_InterruptGlobalDisable(XEntryconv *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XEntryconv_WriteReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_GIE, 0);
}

void XEntryconv_InterruptEnable(XEntryconv *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XEntryconv_ReadReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_IER);
    XEntryconv_WriteReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_IER, Register | Mask);
}

void XEntryconv_InterruptDisable(XEntryconv *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XEntryconv_ReadReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_IER);
    XEntryconv_WriteReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_IER, Register & (~Mask));
}

void XEntryconv_InterruptClear(XEntryconv *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XEntryconv_WriteReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_ISR, Mask);
}

u32 XEntryconv_InterruptGetEnabled(XEntryconv *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XEntryconv_ReadReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_IER);
}

u32 XEntryconv_InterruptGetStatus(XEntryconv *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XEntryconv_ReadReg(InstancePtr->Control_BaseAddress, XENTRYCONV_CONTROL_ADDR_ISR);
}

