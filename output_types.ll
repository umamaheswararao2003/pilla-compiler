; ModuleID = 'pilla-module'
source_filename = "pilla-module"

@0 = private unnamed_addr constant [8 x i8] c"\22hello\22\00", align 1

define i64 @main() {
entry:
  %res = alloca double, align 8
  %c = alloca i8, align 1
  %s = alloca ptr, align 8
  %i = alloca i64, align 8
  %f = alloca double, align 8
  store double 3.140000e+00, ptr %f, align 8
  store i64 10, ptr %i, align 4
  store ptr @0, ptr %s, align 8
  store i8 39, ptr %c, align 1
  %f1 = load double, ptr %f, align 8
  %i2 = load i64, ptr %i, align 4
  %casttmp = sitofp i64 %i2 to double
  %addtmp = fadd double %f1, %casttmp
  store double %addtmp, ptr %res, align 8
  ret i64 0
}
