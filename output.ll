; ModuleID = 'pilla-module'
source_filename = "pilla-module"

@0 = private unnamed_addr constant [12 x i8] c"\22sum is %c\22\00", align 1

define i8 @sum(i8 %a, i8 %b, i8 %c) {
entry:
  %addtmp = add i8 %b, %a
  %addtmp7 = add i8 %addtmp, %c
  ret i8 %addtmp7
}

define i64 @main() {
entry:
  %calltmp = call i8 @sum(i8 10, i8 20, i8 39)
  %calltmp4 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @0, i8 %calltmp)
  ret i64 0
}

declare i32 @printf(ptr, ...)