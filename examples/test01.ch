fn main(): u64 {

    let x = 5u64;

    // variables can be redefined inside the same scope
    let x = if x < 5 {
        break@if 5u64;
    } else if x == 5u64 {
        x + 6 * 10
    } else {
        x + 7 * 10
    };

    x
}