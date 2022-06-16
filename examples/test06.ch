/*
 * This file should not compile. It is an example of wrong program.
 *
*/


fn loop_fun(mut x: i32, mut y: i32): i32 {

    // loop expressions can return any type
    let y = loop {

        if x < y {
            break@loop x; // here we have to specify @loop, otherwise it would break the if-block
        }

        // here y still refers to the parameter
        y = y + 1;
        x = x - 1;
    };

    // this is not valid, because the loop never ends, so the type would be !
    let z = loop {

    };

    // you cannot return any value from a while loop (except for '()')
    let a = while true {
        break 6;
    };

    x * y
}


fn main(): i32 {

    let x = loop_fun(20i32, 10i32);

    x
}
