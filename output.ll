; ModuleID = 'pilla-module'
source_filename = "pilla-module"

@0 = private unnamed_addr constant [12 x i8] c"\22sum is %d\22\00", align 1

define i64 @sum(i64 %a, i64 %b, i64 %c) {
entry:
  %addtmp = add i64 %b, %a
  %addtmp7 = add i64 %addtmp, %c
  ret i64 %addtmp7
}

define i64 @main() {
entry:
  %calltmp = call i64 @sum(i64 10, i64 -12, i64 12)
  %calltmp4 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @0, i64 %calltmp)
  ret i64 0
}

declare i32 @printf(ptr, ...)