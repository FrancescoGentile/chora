; ModuleID = 'examples/test03.ch'
source_filename = "examples/test03.ch"
target triple = "x86_64-pc-linux-gnu"

define i64 @main() {
fn_entry:
  br i1 false, label %then, label %else_if

then:                                             ; preds = %fn_entry
  br label %fn_exit

else_if:                                          ; preds = %fn_entry
  br i1 true, label %then1, label %else

then1:                                            ; preds = %else_if
  br label %fn_exit

else:                                             ; preds = %else_if
  br label %fn_exit

fn_exit:                                          ; preds = %else, %then1, %then
  %0 = phi i64 [ 6, %then ], [ 7, %then1 ], [ 8, %else ]
  ret i64 %0
}
