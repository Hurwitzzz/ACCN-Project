set SynModuleInfo {
  {SRCNAME EntryConv_Pipeline_VITIS_LOOP_8_1 MODELNAME EntryConv_Pipeline_VITIS_LOOP_8_1 RTLNAME EntryConv_EntryConv_Pipeline_VITIS_LOOP_8_1
    SUBMODULES {
      {MODELNAME EntryConv_flow_control_loop_pipe_sequential_init RTLNAME EntryConv_flow_control_loop_pipe_sequential_init BINDTYPE interface TYPE internal_upc_flow_control INSTNAME EntryConv_flow_control_loop_pipe_sequential_init_U}
    }
  }
  {SRCNAME EntryConv_Pipeline_OL MODELNAME EntryConv_Pipeline_OL RTLNAME EntryConv_EntryConv_Pipeline_OL
    SUBMODULES {
      {MODELNAME EntryConv_fadd_32ns_32ns_32_5_full_dsp_1 RTLNAME EntryConv_fadd_32ns_32ns_32_5_full_dsp_1 BINDTYPE op TYPE fadd IMPL fulldsp LATENCY 4 ALLOW_PRAGMA 1}
      {MODELNAME EntryConv_fmul_32ns_32ns_32_4_max_dsp_1 RTLNAME EntryConv_fmul_32ns_32ns_32_4_max_dsp_1 BINDTYPE op TYPE fmul IMPL maxdsp LATENCY 3 ALLOW_PRAGMA 1}
    }
  }
  {SRCNAME EntryConv MODELNAME EntryConv RTLNAME EntryConv IS_TOP 1
    SUBMODULES {
      {MODELNAME EntryConv_gmem_m_axi RTLNAME EntryConv_gmem_m_axi BINDTYPE interface TYPE adapter IMPL m_axi}
      {MODELNAME EntryConv_control_s_axi RTLNAME EntryConv_control_s_axi BINDTYPE interface TYPE interface_s_axilite}
    }
  }
}
