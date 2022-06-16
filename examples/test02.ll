; ModuleID = 'examples/test02.ch'
source_filename = "examples/test02.ch"
target triple = "x86_64-pc-linux-gnu"

%ctor_struct = type { i32, void ()*, i8* }

@global = internal global i64 0
@x_guard = internal global i1 false
@x = internal global i64 0
@llvm.global_ctors = appending global [1 x %ctor_struct] [%ctor_struct { i32 65536, void ()* @ctor_fun, i8* null }]

define void @ctor_fun() {
entry:
  store i64 4, i64* @global, align 4
  ret void
}

define i64 @pluto(i64 %until) {
fn_entry:
  %0 = load i1, i1* @x_guard, align 1
  %1 = icmp eq i1 %0, false
  br i1 %1, label %2, label %3

2:                                                ; preds = %fn_entry
  store i64 8, i64* @x, align 4
  store i1 true, i1* @x_guard, align 1
  br label %3

3:                                                ; preds = %2, %fn_entry
  br label %while_entry

while_entry:                                      ; preds = %while_body, %3
  %4 = load i64, i64* @x, align 4
  %5 = icmp ult i64 %4, %until
  %6 = icmp eq i1 %5, true
  br i1 %5, label %while_body, label %while_after

while_body:                                       ; preds = %while_entry
  %7 = load i64, i64* @x, align 4
  %8 = add i64 %7, 1
  store i64 %8, i64* @x, align 4
  br label %while_entry

while_after:                                      ; preds = %while_entry
  %9 = load i64, i64* @x, align 4
  br label %fn_exit

fn_exit:                                          ; preds = %while_after
  %10 = phi i64 [ %9, %while_after ]
  ret i64 %10
}

define i64 @main() {
fn_entry:
  %0 = call i64 @pluto(i64 25)
  %1 = call i64 @pluto(i64 20)
  %2 = add i64 %0, %1
  br label %fn_exit

fn_exit:                                          ; preds = %fn_entry
  %3 = phi i64 [ %2, %fn_entry ]
  ret i64 %3
}
