fn main(): u8 {

    let z = 500u8;
    // the compiler does not prevent you to declare a number bigger than the max number that can be represented
    // by a type, so this will cause overflow

    let x = 5 + 6 * 10 - 11;
    let y = ((5 + 6) * 10) - 11; // x == y

    // remember to use parenthesis
    // otherwise x < y || x > y would be evaluated as ((x < y) || x ) > y which is not valid
    if (x < y) || (x > y) {
        return@main 10;
    } else {
        return@main z;
    }
}