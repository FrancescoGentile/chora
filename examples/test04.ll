; ModuleID = 'examples/test04.ch'
source_filename = "examples/test04.ch"
target triple = "x86_64-pc-linux-gnu"

define i64 @pluto(i64 %x, i64 %y) {
fn_entry:
  %a = alloca i64, align 8
  store i64 0, i64* %a, align 4
  %b = alloca i64, align 8
  store i64 0, i64* %b, align 4
  br label %while_entry

while_entry:                                      ; preds = %while_after, %fn_entry
  %0 = load i64, i64* %a, align 4
  %1 = icmp slt i64 %0, %x
  %2 = icmp eq i1 %1, true
  br i1 %1, label %while_body, label %while_after4

while_body:                                       ; preds = %while_entry
  br label %while_entry1

while_entry1:                                     ; preds = %if_after, %while_body
  %3 = load i64, i64* %b, align 4
  %4 = icmp slt i64 %3, %y
  %5 = icmp eq i1 %4, true
  br i1 %4, label %while_body2, label %while_after

while_body2:                                      ; preds = %while_entry1
  %6 = load i64, i64* %b, align 4
  %7 = add i64 %6, 1
  store i64 %7, i64* %b, align 4
  %8 = load i64, i64* %b, align 4
  %9 = icmp sgt i64 %8, 20
  br i1 %9, label %then, label %else_if

then:                                             ; preds = %while_body2
  br label %while_after4

else_if:                                          ; preds = %while_body2
  %10 = load i64, i64* %b, align 4
  %11 = icmp eq i64 %10, 20
  br i1 %11, label %then3, label %if_after

then3:                                            ; preds = %else_if
  br label %fn_exit

if_after:                                         ; preds = %else_if
  br label %while_entry1

while_after:                                      ; preds = %while_entry1
  %12 = load i64, i64* %a, align 4
  %13 = add i64 %12, 1
  store i64 %13, i64* %a, align 4
  br label %while_entry

while_after4:                                     ; preds = %then, %while_entry
  %14 = load i64, i64* %a, align 4
  %15 = load i64, i64* %b, align 4
  %16 = add i64 %14, %15
  br label %fn_exit

fn_exit:                                          ; preds = %while_after4, %then3
  %17 = phi i64 [ 10, %then3 ], [ %16, %while_after4 ]
  ret i64 %17
}

define i64 @main() {
fn_entry:
  %0 = call i64 @pluto(i64 10, i64 15)
  %1 = call i64 @pluto(i64 50, i64 10)
  br label %fn_exit

fn_exit:                                          ; preds = %fn_entry
  %2 = phi i64 [ %0, %fn_entry ]
  ret i64 %2
}
