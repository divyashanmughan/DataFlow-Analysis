; ModuleID = 'input.bc'
source_filename = "sample_input/small.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [10 x i32] zeroinitializer, align 16
@g = common global i32 0, align 4

; Function Attrs: nounwind uwtable
define i32 @small(i32 %s) #0 {
entry:
  %mul = mul nsw i32 %s, %s
  %div = sdiv i32 %mul, 2
  %cmp = icmp sgt i32 %s, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %add = add nsw i32 %mul, %div
  %add1 = add nsw i32 %add, %s
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %x.0 = phi i32 [ %add1, %if.then ], [ %mul, %entry ]
  ret i32 %x.0
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.1 (tags/RELEASE_391/final)"}
