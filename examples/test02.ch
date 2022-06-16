
// global static variables are initialized at startup
static global: u64 = 4;

fn pluto(until: u64): u64 {

    // static variables inside a block are initialized only the first time the block is executed
    static mut x: u64 = 5 + 3;

    while x < until {
        x = x + 1;
    }

    x
}

fn main(): u64 {

    // when passing arguments to a function, specify the type of each argument
    // for example, if you wrote pippo(25), this would be interpreted as pippo(25i32)
    // so no function would match
    let x = pluto(25 as u64);
    let y = pluto(20u64);
    x + y
}