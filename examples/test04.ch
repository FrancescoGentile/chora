fn pluto(x: i64, y: i64): i64 {

    let mut a = 0_i64;
    let mut b: i64 = 0;

    while a < x @out: {

        while b < y {
            b = b + 1; // operator += not implemented

            if b > 20 {
                return@while; // returns from the outermost while
                // break@out; this is equivalent
                // return@out; this is equivalent
                // break; this is not equivalent -> this breaks the innermost while loop
            } else if b == 20 {
                //return@pluto 10; // return from the function
                // return 10; this is equivalent
                break@pluto 10; // this is equivalent
            }
        }

        a = a + 1;
    }

    a + b
}

fn main(): i64 {

    let x = pluto(x = 10i64, y = 15i64);
    let y = pluto(y = 10i64, x = 50i64); // you can specify named arguments in any order

    x
}