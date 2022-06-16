; ModuleID = 'examples/test05.ch'
source_filename = "examples/test05.ch"
target triple = "x86_64-pc-linux-gnu"

define i64 @dist_return(i1 %a, i64 %b, i32 %c, i64 %d) {
fn_entry:
  br label %fn_exit

fn_exit:                                          ; preds = %fn_entry
  %0 = phi i64 [ %b, %fn_entry ]
  ret i64 %0
}

define i32 @dist_return.1(i1 %a, i64 %b, i32 %c, i64 %d) {
fn_entry:
  %0 = trunc i64 %b to i32
  br label %fn_exit

fn_exit:                                          ; preds = %fn_entry
  %1 = phi i32 [ %0, %fn_entry ]
  ret i32 %1
}

define void @dist_return.2(i1 %a) {
fn_entry:
  br label %fn_exit

fn_exit:                                          ; preds = %fn_entry
  ret void
}

define void @main() {
fn_entry:
  %0 = call i64 @dist_return(i1 true, i64 5, i32 4, i64 6)
  call void @dist_return.2(i1 false)
  br label %fn_exit

fn_exit:                                          ; preds = %fn_entry
  ret void
}
