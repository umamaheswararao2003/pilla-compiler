; ModuleID = 'pilla-module'
source_filename = "pilla-module"

@0 = private unnamed_addr constant [5 x i8] c"\22%d\22\00", align 1
@1 = private unnamed_addr constant [5 x i8] c"\22%d\22\00", align 1

define i64 @sum(i64 %a, i64 %b, i64 %c) {
entry:
  %addtmp = add i64 %b, %a
  %addtmp7 = add i64 %addtmp, %c
  ret i64 %addtmp7
}

define void @main() {
entry:
  %calltmp = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @0, i64 20)
  %calltmp4 = call i64 @sum(i64 10, i64 20, i64 3)
  %calltmp5 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @1, i64 %calltmp4)
  ret void
}

declare i32 @printf(ptr, ...)