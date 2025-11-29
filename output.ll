; ModuleID = 'pilla-module'
source_filename = "pilla-module"

define i64 @main() {
entry:
  %c = alloca i64, align 8
  %b = alloca i64, align 8
  %a = alloca i64, align 8
  store i64 10, ptr %a, align 4
  store i64 20, ptr %b, align 4
  %a1 = load i64, ptr %a, align 4
  %b2 = load i64, ptr %b, align 4
  %addtmp = add i64 %a1, %b2
  store i64 %addtmp, ptr %c, align 4
  %c3 = load i64, ptr %c, align 4
  ret i64 %c3
}
