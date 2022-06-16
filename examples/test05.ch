fn dist_return(a: bool = true, b: u64, c: i32 = 4, d: u64): u64 {
    b
}

fn dist_return(a: bool = true, b: u64, c: i32 = 4, d: u64): u32 {
    b as u32
}

fn dist_return(a: bool = false) {

}

fn main() {

    /*
     * Here the first two functions have the same parameter list,
     * but different return type.
     */

    // if we didn't specify the type of x, the compiler could not select the right function
    // because both are valid
    let x: u64 = dist_return(b = 5u64, d = 6u64);

    // after a named argument, the following non-named argument must correspond
    // to the parameters following the named one in the parameter list
    //let y: u32 = dist_return(b = 5u64, 5i32, 5u64);

    let z: () = dist_return();
}