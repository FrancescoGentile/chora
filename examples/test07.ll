; ModuleID = 'examples/test07.ch'
source_filename = "examples/test07.ch"
target triple = "x86_64-pc-linux-gnu"

define i8 @main() {
fn_entry:
  br i1 false, label %or_final, label %or_false

or_false:                                         ; preds = %fn_entry
  br label %or_final

or_final:                                         ; preds = %or_false, %fn_entry
  %0 = phi i1 [ true, %fn_entry ], [ false, %or_false ]
  br i1 %0, label %then, label %else

then:                                             ; preds = %or_final
  br label %fn_exit

else:                                             ; preds = %or_final
  br label %fn_exit

fn_exit:                                          ; preds = %else, %then
  %1 = phi i8 [ 10, %then ], [ -12, %else ]
  ret i8 %1
}
