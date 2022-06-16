; ModuleID = 'examples/test01.ch'
source_filename = "examples/test01.ch"
target triple = "x86_64-pc-linux-gnu"

define i64 @main() {
fn_entry:
  br i1 false, label %then, label %else_if

then:                                             ; preds = %fn_entry
  br label %if_after

else_if:                                          ; preds = %fn_entry
  br i1 true, label %then1, label %else

then1:                                            ; preds = %else_if
  br label %if_after

else:                                             ; preds = %else_if
  br label %if_after

if_after:                                         ; preds = %else, %then1, %then
  %0 = phi i64 [ 5, %then ], [ 110, %then1 ], [ 120, %else ]
  br label %fn_exit

fn_exit:                                          ; preds = %if_after
  %1 = phi i64 [ %0, %if_after ]
  ret i64 %1
}
