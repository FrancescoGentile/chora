//
// Created by francesco on 28/11/21.
//

#ifndef CHORA_STYLE_H
#define CHORA_STYLE_H

// ref: https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal

/*
 *          foreground background
 *  black        30         40
 *  red          31         41
 *  green        32         42
 *  yellow       33         43
 *  blue         34         44
 *  magenta      35         45
 *  cyan         36         46
 *  white        37         47
 */

/*
 * reset             0  (everything back to normal)
 * bold/bright       1  (often a brighter shade of the same colour)
 * underline         4
 * inverse           7  (swap foreground and background colours)
 * bold/bright off  21
 * underline off    24
 * inverse off      27
 */

#define RESET "\033[0m"

#define BOLD "\033[1m"

#define RED "\033[31m"

#define YELLOW "\033[33m"

#define BLUE "\033[34m"

#define WHITE "\033[37m"

#endif //CHORA_STYLE_H
