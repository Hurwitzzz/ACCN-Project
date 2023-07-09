; ModuleID = '/home/clyybber/projects/uni/cnnhw/final/ACCN-Homework/workflow/vitis_workflow2/solution1/.autopilot/db/a.g.ld.5.gdce.bc'
source_filename = "llvm-link"
target datalayout = "e-m:e-i64:64-i128:128-i256:256-i512:512-i1024:1024-i2048:2048-i4096:4096-n8:16:32:64-S128-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "fpga64-xilinx-none"

; Function Attrs: inaccessiblemem_or_argmemonly noinline
define void @apatb_EntryConv_ir(float* noalias nocapture nonnull readonly "fpga.decayed.dim.hint"="100" "maxi" %X, float* noalias nocapture nonnull readonly "fpga.decayed.dim.hint"="3" "maxi" %W, float* noalias nocapture nonnull "fpga.decayed.dim.hint"="98" "maxi" %Z) local_unnamed_addr #0 {
entry:
  %X_copy = alloca [100 x float], align 512
  %W_copy = alloca [3 x float], align 512
  %Z_copy = alloca [98 x float], align 512
  %0 = bitcast float* %X to [100 x float]*
  %1 = bitcast float* %W to [3 x float]*
  %2 = bitcast float* %Z to [98 x float]*
  call fastcc void @copy_in([100 x float]* nonnull %0, [100 x float]* nonnull align 512 %X_copy, [3 x float]* nonnull %1, [3 x float]* nonnull align 512 %W_copy, [98 x float]* nonnull %2, [98 x float]* nonnull align 512 %Z_copy)
  call void @apatb_EntryConv_hw([100 x float]* %X_copy, [3 x float]* %W_copy, [98 x float]* %Z_copy)
  call void @copy_back([100 x float]* %0, [100 x float]* %X_copy, [3 x float]* %1, [3 x float]* %W_copy, [98 x float]* %2, [98 x float]* %Z_copy)
  ret void
}

; Function Attrs: argmemonly noinline norecurse
define internal fastcc void @copy_in([100 x float]* noalias readonly, [100 x float]* noalias align 512, [3 x float]* noalias readonly, [3 x float]* noalias align 512, [98 x float]* noalias readonly, [98 x float]* noalias align 512) unnamed_addr #1 {
entry:
  call fastcc void @onebyonecpy_hls.p0a100f32([100 x float]* align 512 %1, [100 x float]* %0)
  call fastcc void @onebyonecpy_hls.p0a3f32([3 x float]* align 512 %3, [3 x float]* %2)
  call fastcc void @onebyonecpy_hls.p0a98f32([98 x float]* align 512 %5, [98 x float]* %4)
  ret void
}

; Function Attrs: argmemonly noinline norecurse
define internal fastcc void @onebyonecpy_hls.p0a100f32([100 x float]* noalias align 512 %dst, [100 x float]* noalias readonly %src) unnamed_addr #2 {
entry:
  %0 = icmp eq [100 x float]* %dst, null
  %1 = icmp eq [100 x float]* %src, null
  %2 = or i1 %0, %1
  br i1 %2, label %ret, label %copy

copy:                                             ; preds = %entry
  call void @arraycpy_hls.p0a100f32([100 x float]* nonnull %dst, [100 x float]* nonnull %src, i64 100)
  br label %ret

ret:                                              ; preds = %copy, %entry
  ret void
}

; Function Attrs: argmemonly noinline norecurse
define void @arraycpy_hls.p0a100f32([100 x float]* %dst, [100 x float]* readonly %src, i64 %num) local_unnamed_addr #3 {
entry:
  %0 = icmp eq [100 x float]* %src, null
  %1 = icmp eq [100 x float]* %dst, null
  %2 = or i1 %1, %0
  br i1 %2, label %ret, label %copy

copy:                                             ; preds = %entry
  %for.loop.cond1 = icmp sgt i64 %num, 0
  br i1 %for.loop.cond1, label %for.loop.lr.ph, label %copy.split

for.loop.lr.ph:                                   ; preds = %copy
  br label %for.loop

for.loop:                                         ; preds = %for.loop, %for.loop.lr.ph
  %for.loop.idx2 = phi i64 [ 0, %for.loop.lr.ph ], [ %for.loop.idx.next, %for.loop ]
  %dst.addr = getelementptr [100 x float], [100 x float]* %dst, i64 0, i64 %for.loop.idx2
  %src.addr = getelementptr [100 x float], [100 x float]* %src, i64 0, i64 %for.loop.idx2
  %3 = load float, float* %src.addr, align 4
  store float %3, float* %dst.addr, align 4
  %for.loop.idx.next = add nuw nsw i64 %for.loop.idx2, 1
  %exitcond = icmp ne i64 %for.loop.idx.next, %num
  br i1 %exitcond, label %for.loop, label %copy.split

copy.split:                                       ; preds = %for.loop, %copy
  br label %ret

ret:                                              ; preds = %copy.split, %entry
  ret void
}

; Function Attrs: argmemonly noinline norecurse
define internal fastcc void @onebyonecpy_hls.p0a3f32([3 x float]* noalias align 512 %dst, [3 x float]* noalias readonly %src) unnamed_addr #2 {
entry:
  %0 = icmp eq [3 x float]* %dst, null
  %1 = icmp eq [3 x float]* %src, null
  %2 = or i1 %0, %1
  br i1 %2, label %ret, label %copy

copy:                                             ; preds = %entry
  call void @arraycpy_hls.p0a3f32([3 x float]* nonnull %dst, [3 x float]* nonnull %src, i64 3)
  br label %ret

ret:                                              ; preds = %copy, %entry
  ret void
}

; Function Attrs: argmemonly noinline norecurse
define void @arraycpy_hls.p0a3f32([3 x float]* %dst, [3 x float]* readonly %src, i64 %num) local_unnamed_addr #3 {
entry:
  %0 = icmp eq [3 x float]* %src, null
  %1 = icmp eq [3 x float]* %dst, null
  %2 = or i1 %1, %0
  br i1 %2, label %ret, label %copy

copy:                                             ; preds = %entry
  %for.loop.cond1 = icmp sgt i64 %num, 0
  br i1 %for.loop.cond1, label %for.loop.lr.ph, label %copy.split

for.loop.lr.ph:                                   ; preds = %copy
  br label %for.loop

for.loop:                                         ; preds = %for.loop, %for.loop.lr.ph
  %for.loop.idx2 = phi i64 [ 0, %for.loop.lr.ph ], [ %for.loop.idx.next, %for.loop ]
  %dst.addr = getelementptr [3 x float], [3 x float]* %dst, i64 0, i64 %for.loop.idx2
  %src.addr = getelementptr [3 x float], [3 x float]* %src, i64 0, i64 %for.loop.idx2
  %3 = load float, float* %src.addr, align 4
  store float %3, float* %dst.addr, align 4
  %for.loop.idx.next = add nuw nsw i64 %for.loop.idx2, 1
  %exitcond = icmp ne i64 %for.loop.idx.next, %num
  br i1 %exitcond, label %for.loop, label %copy.split

copy.split:                                       ; preds = %for.loop, %copy
  br label %ret

ret:                                              ; preds = %copy.split, %entry
  ret void
}

; Function Attrs: argmemonly noinline norecurse
define internal fastcc void @onebyonecpy_hls.p0a98f32([98 x float]* noalias align 512 %dst, [98 x float]* noalias readonly %src) unnamed_addr #2 {
entry:
  %0 = icmp eq [98 x float]* %dst, null
  %1 = icmp eq [98 x float]* %src, null
  %2 = or i1 %0, %1
  br i1 %2, label %ret, label %copy

copy:                                             ; preds = %entry
  call void @arraycpy_hls.p0a98f32([98 x float]* nonnull %dst, [98 x float]* nonnull %src, i64 98)
  br label %ret

ret:                                              ; preds = %copy, %entry
  ret void
}

; Function Attrs: argmemonly noinline norecurse
define void @arraycpy_hls.p0a98f32([98 x float]* %dst, [98 x float]* readonly %src, i64 %num) local_unnamed_addr #3 {
entry:
  %0 = icmp eq [98 x float]* %src, null
  %1 = icmp eq [98 x float]* %dst, null
  %2 = or i1 %1, %0
  br i1 %2, label %ret, label %copy

copy:                                             ; preds = %entry
  %for.loop.cond1 = icmp sgt i64 %num, 0
  br i1 %for.loop.cond1, label %for.loop.lr.ph, label %copy.split

for.loop.lr.ph:                                   ; preds = %copy
  br label %for.loop

for.loop:                                         ; preds = %for.loop, %for.loop.lr.ph
  %for.loop.idx2 = phi i64 [ 0, %for.loop.lr.ph ], [ %for.loop.idx.next, %for.loop ]
  %dst.addr = getelementptr [98 x float], [98 x float]* %dst, i64 0, i64 %for.loop.idx2
  %src.addr = getelementptr [98 x float], [98 x float]* %src, i64 0, i64 %for.loop.idx2
  %3 = load float, float* %src.addr, align 4
  store float %3, float* %dst.addr, align 4
  %for.loop.idx.next = add nuw nsw i64 %for.loop.idx2, 1
  %exitcond = icmp ne i64 %for.loop.idx.next, %num
  br i1 %exitcond, label %for.loop, label %copy.split

copy.split:                                       ; preds = %for.loop, %copy
  br label %ret

ret:                                              ; preds = %copy.split, %entry
  ret void
}

; Function Attrs: argmemonly noinline norecurse
define internal fastcc void @copy_out([100 x float]* noalias, [100 x float]* noalias readonly align 512, [3 x float]* noalias, [3 x float]* noalias readonly align 512, [98 x float]* noalias, [98 x float]* noalias readonly align 512) unnamed_addr #4 {
entry:
  call fastcc void @onebyonecpy_hls.p0a100f32([100 x float]* %0, [100 x float]* align 512 %1)
  call fastcc void @onebyonecpy_hls.p0a3f32([3 x float]* %2, [3 x float]* align 512 %3)
  call fastcc void @onebyonecpy_hls.p0a98f32([98 x float]* %4, [98 x float]* align 512 %5)
  ret void
}

declare void @apatb_EntryConv_hw([100 x float]*, [3 x float]*, [98 x float]*)

; Function Attrs: argmemonly noinline norecurse
define internal fastcc void @copy_back([100 x float]* noalias, [100 x float]* noalias readonly align 512, [3 x float]* noalias, [3 x float]* noalias readonly align 512, [98 x float]* noalias, [98 x float]* noalias readonly align 512) unnamed_addr #4 {
entry:
  call fastcc void @onebyonecpy_hls.p0a98f32([98 x float]* %4, [98 x float]* align 512 %5)
  ret void
}

define void @EntryConv_hw_stub_wrapper([100 x float]*, [3 x float]*, [98 x float]*) #5 {
entry:
  call void @copy_out([100 x float]* null, [100 x float]* %0, [3 x float]* null, [3 x float]* %1, [98 x float]* null, [98 x float]* %2)
  %3 = bitcast [100 x float]* %0 to float*
  %4 = bitcast [3 x float]* %1 to float*
  %5 = bitcast [98 x float]* %2 to float*
  call void @EntryConv_hw_stub(float* %3, float* %4, float* %5)
  call void @copy_in([100 x float]* null, [100 x float]* %0, [3 x float]* null, [3 x float]* %1, [98 x float]* null, [98 x float]* %2)
  ret void
}

declare void @EntryConv_hw_stub(float*, float*, float*)

attributes #0 = { inaccessiblemem_or_argmemonly noinline "fpga.wrapper.func"="wrapper" }
attributes #1 = { argmemonly noinline norecurse "fpga.wrapper.func"="copyin" }
attributes #2 = { argmemonly noinline norecurse "fpga.wrapper.func"="onebyonecpy_hls" }
attributes #3 = { argmemonly noinline norecurse "fpga.wrapper.func"="arraycpy_hls" }
attributes #4 = { argmemonly noinline norecurse "fpga.wrapper.func"="copyout" }
attributes #5 = { "fpga.wrapper.func"="stub" }

!llvm.dbg.cu = !{}
!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3}
!blackbox_cfg = !{!4}

!0 = !{!"clang version 7.0.0 "}
!1 = !{i32 2, !"Dwarf Version", i32 4}
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{}
