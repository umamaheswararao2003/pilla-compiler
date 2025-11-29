; ModuleID = 'pilla-module'
source_filename = "pilla-module"

define i64 @sum(i64 %a, i64 %b, i64 %c) {
entry:
  %a1 = alloca i64, align 8
  store i64 %a, ptr %a1, align 4
  %b2 = alloca i64, align 8
  store i64 %b, ptr %b2, align 4
  %c3 = alloca i64, align 8
  store i64 %c, ptr %c3, align 4
  %a4 = load i64, ptr %a1, align 4
  %b5 = load i64, ptr %b2, align 4
  %addtmp = add i64 %a4, %b5
  %c6 = load i64, ptr %c3, align 4
  %addtmp7 = add i64 %addtmp, %c6
  ret i64 %addtmp7
}

define i64 @main() {
entry:
  %c = alloca i64, align 8
  %b = alloca i64, align 8
  %a = alloca i64, align 8
  store i64 10, ptr %a, align 4
  store i64 20, ptr %b, align 4
  %a1 = load i64, ptr %a, align 4
  %b2 = load i64, ptr %b, align 4
  %calltmp = call i64 @sum(i64 %a1, i64 %b2, i64 10)
  store i64 %calltmp, ptr %c, align 4
  ret i64 0
}