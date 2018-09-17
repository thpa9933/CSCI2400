/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2017, fifth edition, plus
   the following additions from Amendment 1 to the fifth edition:
   - 56 emoji characters
   - 285 hentaigana
   - 3 additional Zanabazar Square characters */
/* We do not support C11 <threads.h>.  */

/*DEC VARS ON TOP OF FUNCTION*/


/* 
 * bitOr - x|y using only ~ and & 
 *   Example: bitOr(6, 5) = 7
 *   Legal ops: ~ &
 *   Max ops: 8
 *   Rating: 1

 De'Morgans! not(p or q) bicond (not p and not q)
 Negate ans to account for the <not>(p or q)
 */
int bitOr(int x, int y) {
  int ans; 
  ans = (~x & ~y); 
  return ~ans; 
} 


/*
 * evenBits - return word with all even-numbered bits set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 1
  Find an even bit pattern in hex, 01010101... is 0x55 (1 byte)
  Then left shift by 8, so you have 01010101000000000 (2 bytes)
  If you do | between those values, the final value will be 0101010101010101
  Then shift it over 16, which will pad right with 16 0's, use or to retain the prior value (4bytes)
 */
int evenBits(void) {
  int a;
  a = 0x55; 
  a = a | a << 8;
  a = a | a << 16; 
  return a; 
}


/* 
 * minusOne - return a value of -1 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max :ops: 2
 *   Rating: 1
 0xff == 1111 = -1 in two's compliment
 0x00 = 0000, ~0x00 = 1111 = -1 in two's compliment
 */
int minusOne(void) {
  int max;
  max = 0x00; 
  return ~max;
}


/* 
 * allEvenBits - return 1 if all even-numbered bits in word set to 1
 *   Examples allEvenBits(0xFFFFFFFE) = 0, allEvenBits(0x55555555) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allEvenBits(int x) {
  int oddBits;
  int allBits;
  oddBits = 0xAA;
  oddBits = oddBits | oddBits << 8; 
  oddBits = oddBits | oddBits << 16; // word with all odd bits

  // will be a 1 at every odd bit, should be 1 at every even bit too if x has 1 at every even bit
  allBits = oddBits | x;
  
  allBits = ~allBits; // should be all 0's if x had each even bit as a 1
  return !allBits; // !000.. is 1 (! is binary (returns 1 or 0). ~ is unitary, returns 'flipped' word
}


/* 
 * anyOddBit - return 1 if any odd-numbered bit in word set to 1
 *   Examples anyOddBit(0x5) = 0, anyOddBit(0x7) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int anyOddBit(int x) {
  int y, ans;
  y = 0xAA; 
  y = y | y << 8; 
  y = y | y << 16; // string of 1010
  ans = y & x; // if x contains a 1 as an odd bit, then the result will be ex: 0010, otherwise all 0's
  // !0010 = 0, !0 = 1;
  // If result was 0000, !0000 = 1, !1 == 0 = false
  // !(anything but 0000) = 0  
  return !(!ans); 
}


/* 
 * byteSwap - swaps the nth byte and the mth byte
 *  Examples: byteSwap(0x12345678, 1, 3) = 0x56341278
 *            byteSwap(0xDEADBEEF, 0, 2) = 0xDEEFBEAD
 *  You may assume that 0 <= n <= 3, 0 <= m <= 3
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 2
 */
int byteSwap(int x, int n, int m) {
  int new_n, new_m, mask_1, mask_2, total_mask, mask_n, mask_m, temp_mask, final_mask, ans;
  n = n << 3; // 2^3 = 8;
  m = m << 3; // locates where bytes are in sittingg

  new_n = x >> n; // shifts the located bites to far right
  new_m = x >> m;

  // 0xff = 0000 0000 0000 .... 1111 1111
  // 0xff & anything = make resulting string all 0's
  new_n = 0xff & new_n; // pads evertyhing to the left of located bites with 0's 
  new_m = 0xff & new_m; // the 0xff at the end, will allow new_n to fall through

  // create new masks with same selected bit locations, with all 1's
  mask_1 = 0xff << n; //ex: 0000 0000 1111 1111 0000 0000 ....
  mask_2 = 0xff << m; //ex: 0000 0000 0000 0000 1111 1111 ....

  // flip the bytes so located bits are 0's
  // 1111 1111 0000 0000 1111 1111 ...
  // you 'and' total_mask because you want 0's to be in the located positions
  temp_mask = ~mask_1 & ~mask_2;
  total_mask = x & temp_mask;
 
  // swap the bytes, remember 'm' and 'n' are location of the bytes
  mask_n = new_n << m;
  mask_m = new_m << n;

  // make a final word with locations swapped
  // ex: 0000 0000 1011 0011 0000 0000 1010 1101 
  final_mask = mask_n | mask_m;

  // this allows the remaining values that were never swapped to fall through
  ans = final_mask | total_mask; 
  return ans; 
}


/* 
 * addOK - Determine if can compute x+y without overflow
 *   Example: addOK(0x80000000,0x80000000) = 0,
 *            addOK(0x80000000,0x70000000) = 1, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int addOK(int x, int y) {
  int sign_x, sign_y, sign_sum, cond;
 // determine the sign of the ints by shifting over so only 32 position is available;
 sign_x = x >> 31;
 sign_y = y >> 31;
 sign_sum = x + y;
 sign_sum = sign_sum >> 31;

 // check condtions
 // sign_x = sign_y but sign_sum is not (always overflow) (CHECKING THIS CONDITION)
 // sign_x = sign_y but sign_sum is (not overflow)
 // sign_x != sign_y (never overflow)

 // use ^ because it returns 1 if the are different
 // (-x ^ y) checks if equal
 // returns 0 if there is overflow 
 cond = !((~sign_x ^ sign_y) & (sign_x ^ sign_sum));
 return cond;
}


/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
// if x evaluates to T return y
// if x evaluates to F return z
int conditional(int x, int y, int z) {
  int mask;
  // ~0x00 is all 1's
  // !x will turn x into a 1 or a 0 EX: !1011 = 0 , !0000 = 1;
  // mask will be greater than all 1's if x > 0
  // ~0x00 because that will be 1111 1111, where as 0xff is 0000 1111
  mask = !x + (~0x00); // if x is 0's, then !x = 1. 1 + all 1's = 0
  
  // if x is 1, then 1 + all 1's overflows and becomes all 0's
  // EX: if ~mask = 0000 ...
     // 0000 & anything = 0000, left condition will be F, right condtion will return
  //EX: if ~mask = 1111 ...
     // 1111 & antyhing = anything, left condtion will be anything
     // left condition will return because right condtion will be 0000
  return ((~mask & z) | (mask & y));
}


/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  int upper, lower, upperdiff, lowerdiff;
  // make result negative if outside of bounds
  upper = 0x39;
  lower = 0x30;

  // set the bounds for upper and lower
  // upperdiff equates to upper + (twos compliment of x ) + 1
  // lowerdiff equates to x + (twos compliment of lower) + 1
  // ~10 = -11, so you need to add 1
  upperdiff = upper + (~x + 1); // differnce between x and the upper limit, positive if in range, negative if out of range
  lowerdiff = x + (~lower + 1); // difference between x and the lower limit, positive if in range, negative if out or range
 
  // if the sign bits are both 0, they have positive values 
  return (!(upperdiff >> 31)) & (!(lowerdiff >> 31));
}


/* 
 * replaceByte(x,n,c) - Replace byte n in x with c
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: replaceByte(0x12345678,1,0xab) = 0x1234ab78
 *   You can assume 0 <= n <= 3 and 0 <= c <= 255
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 3
 */
int replaceByte(int x, int n, int c) {
  int mask_ff, mask_c, mask;
  n = n << 3;

  // ex: 1111 1111 0000 0000 1111 1111 1111 ...
  mask_ff = ~(0xff << n);

  // ex: 0000 0000 cccc cccc 0000 0000 0000 ...
  mask_c = c << n;
 
  // allows our data to fall through
  // ex: xxxx xxxx 0000 0000 xxxx xxxx xxxx ....
  mask = x & mask_ff;

  // allows day to fall through, now with the c bits
  return mask | mask_c;
}


/* reverseBits - reverse the bits in a 32-bit integer,
              i.e. b0 swaps with b31, b1 with b30, etc
 *  Examples: reverseBits(0x11111111) = 0x88888888
 *            reverseBits(0xdeadbeef) = 0xf77db57b
 *            reverseBits(0x88888888) = 0x11111111
 *            reverseBits(0)  = 0
 *            reverseBits(-1) = -1
 *            reverseBits(0x9) = 0x90000000
 *  Legal ops: ! ~ & ^ | + << >> and unsigned int type
 *  Max ops: 90
 *  Rating: 4
 */
int reverseBits(int x) {
  int mask1, mask2, mask4, mask8, mask16;

  //0x55 = 01010101 01010101 01010101 01010101
  mask1 = 0x55;
  mask1 = mask1 | mask1 << 8;
  mask1 = mask1 | mask1 << 16;
  // first cond shifts all the even X bits into the odd position, where the mask is a 1
  // those bits are then retained by being 'anded' by a 1

  // second cond already has the odd X bits masked with 1's, so there values are retained
  // the odd x bits are then shifted into the even positions
  x = ((x >> 1) & mask1) | ((x & mask1) << 1);

  //0x33 = 00110011 00110011 00110011 00110011
  mask2 = 0x33;
  mask2 = mask2 | mask2 << 8;
  mask2 = mask2 | mask2 << 16;
  x = ((x >> 2) & mask2) | ((x & mask2) << 2);
  

  //0xF = 00001111 00001111 00001111 00001111
  mask4 = 0xF;
  mask4 = mask4 | mask4 << 8;
  mask4 = mask4 | mask4 << 16;
  x = ((x >> 4) & mask4) | ((x & mask4) << 4);

  // 0xFF = 0000 0000 1111 1111
  // changing into this form after shifiting 16

  // 0xFF = 00000000 11111111 00000000 11111111
  mask8 =0xFF;
  mask8 = mask8 | mask8 << 16;
  x = ((x >> 8) & mask8) | ((x & mask8) << 8);

  // 0xFF = 00000000 00000000 11111111 11111111
  mask16 = 0xFF;
  mask16 = mask16 | mask16 << 8;
  x = ((x >> 16) & mask16) | ((x & mask16) << 16);

  return x;
}



/*
 * satAdd - adds two numbers but when positive overflow occurs, returns
 *          maximum possible value, and when negative overflow occurs,
 *          it returns minimum positive value.
 *   Examples: satAdd(0x40000000,0x40000000) = 0x7fffffff
 *             satAdd(0x80000000,0xffffffff) = 0x80000000
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 30
 *   Rating: 4
 */
int satAdd(int x, int y) {
  int sign_x, sign_y, sign_sum, mask1, lowest, highest, final, sum, overflow;;
  sign_x = x >> 31;
  sign_y = y >> 31;
  sum = x + y;
  sign_sum = sum >> 31;
  
  //EX: sign_x = 0 (positive)
  //EX: sign_y = 0 (positive)

  // Returns 0 if there is overflow
  overflow = !((~sign_x ^ sign_y) & (sign_x ^ sign_sum));
  overflow = ~overflow + 1; // changes back to all 1's or all 0's
  // EX: There IS overflow, result = 0000

  // When you add two postive numbers and get a negative result its postive overflow

  // Figure out if the overflow is + or -
  // determines if x is 1 + 1111 = 0000(overflow) or 0 + 1111 (not overflow)
  mask1 = !!sign_x; // change to 1 or 0 EX: if sign_x is 1111, !sign_x = 0, !!sign_x = 1
  mask1 = (~mask1) + 1; // changes back to all 1's or 0's, neg val is 1 less than pos, so add 1
  // Turns the sign of x into mask. All 1's if x is positive, and all 0's if x is negative.
  // Want mask to be all 0's
  // EX: There IS overflow, want ~mask = 1's in order to we return highest

  // make bits to return
  lowest = 0x1;
  // 0x1 = 1000 0000 0000 0000
  lowest = lowest << 31;
  // ~0x1 = 0111 1111 1111 1111
  highest = ~lowest;
 
  // First Cond: result = 0 if there is overflow, (0 | anthing) = anything
  final = (overflow & sum) | (~overflow & ((mask1 & lowest) | (~mask1 & highest)));
  return final;
}


/*
 * Extra credit
 */
/* 
 * float_abs - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument..
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_abs(unsigned uf) {
  return 2;
}
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
  return 2;
}
/* 
 * float_half - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_half(unsigned uf) {
  return 2;
}
