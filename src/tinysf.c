/* Fill in your Name and GNumber in the following two comment fields
 * Name: Justin Thomas
 * GNumber: 01336576
 */

#include <stdio.h>
#include <stdlib.h>
#include "common_structs.h"
#include "common_definitions.h"
#include "common_functions.h"
#include "tinysf.h"

#define Sign (1 << 12)
#define pos_infinity ((unsigned int) 0x00000f00)
#define neg_infinity ((unsigned int) 0x00001f00)
#define NaN ((unsigned int) 0x00000f10)
#define firstFrac ((unsigned int) (1 << 31))
#define Bias 7
#define zero (unsigned int) 0
#define neg_zero (unsigned int) 0x00001000
#define guardBit ((unsigned int) (1<<24))
#define roundBit ((unsigned int) (1<<23))

// Feel free to add many Helper Functions, Consts, and Definitions!

/*Helper function to shift whole number*/
int wholeShifter(unsigned int whole, unsigned int frac) {
  int E;
  E = 0;

  while(whole != 1) {
    
    if((whole & 1) == 1) {
      frac = frac >> 1;
      frac = frac | firstFrac;
    } else {
      frac = frac >> 1;
    }

    whole = whole >> 1;
    E++;
  }

  return E;

}
/*Helper function to get frac field*/
unsigned int wholeShifter2(unsigned int whole, unsigned int frac) {

  while(whole != 1) {
    
    if((whole & 1) == 1) {
      frac = frac >> 1;
      frac = frac | firstFrac;
    } else {
      frac = frac >> 1;
    }

    whole = whole >> 1;
  }

  return frac;

}

/*Helper function to set exp field  */
unsigned int expSet(int exp) {
  
  unsigned int FPexp;
  FPexp = 0;

  FPexp = (FPexp | exp);

  FPexp = (FPexp << 8);

  return FPexp;
}

/* Helper function to help round up the number*/
unsigned int roundUp(unsigned int number){

  unsigned int carry = (1 << 24);

  while((carry & number) != 0) {/*Loop through to carry the 1 until a 0 is found*/
    number = number & (~(carry));
    
    if(carry == firstFrac) {/*Check the case if entire frac field is 1*/
      return number;
    }

    carry = carry << 1;
  }

  number = number | carry;
  return number;


}

/*Helper Function to round the bits in the number struct*/
unsigned int fracRounder(unsigned int number) {

  unsigned int clear = 0xff000000; /*Used to Clear bits 23-0 in frac field*/
  
  if((((guardBit & number) != 0) && ((roundBit & number))) != 0) {
    number = (number & clear); 
    number = roundUp(number);
    return number;
  }

  if((((guardBit & number) == 0) && ((roundBit & number) == 0)) || (((guardBit & number) != 0) && ((roundBit & number) == 0))) {
    number = (number & clear);
    return number;
  }

  if((((guardBit & number) == 0) && ((roundBit & number)) != 0)) {
    
    if((number & 0x007fffff) == 0) {
      number = (number & clear);
      return number;
    } else {
      number = (number & clear);
      number = roundUp(number);

    }

  } 

  return number;

}

/*Helper function to set the frac field in the 8 bit field*/
unsigned int fracSet(unsigned int number) {

  unsigned int FPfrac;
  FPfrac = 0;

  FPfrac = (FPfrac | number);

  FPfrac = (FPfrac >> 24);

  return FPfrac;

}

/* 
 * Helper function to shift a number that has no whole number and return the exponent (amount of shifts)
 */
int zeroShifter(unsigned int frac){
  int E, found;
  found = 0;
  E = 0;

  while(found == 0) {/*Loop through and shift the frac left the 1 bit is found*/
    
    if((frac & firstFrac) == 0) {
      frac = (frac << 1);
      E--; 
    
    } else {
      found = 1;
      frac = (frac << 1);
      E--;
    }
  }

  return E;
}


/* Helper function to check if any flags are present and apply the necessary return value 
 */
tinysf_s flagChecker(Number_s *number) {

  if ((number->is_infinity == 1) && (number->is_nan == 1)) {
    return NaN;
  }

  else if(number->is_nan == 1){
    return NaN;
  }
  
  else if(number->is_infinity == 1) {

    if(number->is_negative == 1) {
      return neg_infinity;
    } else {
      return pos_infinity;
    }

  }

  return 0;
  

} 

// ----------Public API Functions (write these!)-------------------

/* toTinySF - Converts a Number Struct (whole and fraction parts) into a TINYSF Value
 *  - number is managed by zuon, DO NOT FREE number.
 *  - Follow the project documentation for this function.
 * Return the TINYSF Value.
 */
tinysf_s toTinySF(Number_s *number) {
  int E; /*Exponent*/
  tinysf_s result;
  result = (unsigned int) 0;
  unsigned int fracCopy; /*copy of fraction field to not ruin original number->fraction*/
  unsigned int wholeCopy; /*copy of whole field to not ruin original number->fraction*/
  E = 0;
  
  wholeCopy = number->whole;
  fracCopy = number->fraction;

  result = flagChecker(number);

  if(result != 0) {/*Return NaN or Infinity based on flagChecker*/
    return result;
  }

  if((number->whole == 0) && (number->fraction == 0)) {/*Check if number is 0*/
    
    if(number->is_negative == 0) {
      return (unsigned int) zero;
    } else {
      return (unsigned int) neg_zero;
    }
  }

  if(number->whole == 0) {/*Check if the whole number is 0 to call the function to shift frac to the left*/
    E = zeroShifter(fracCopy);
    fracCopy = fracRounder(fracCopy);
    fracCopy = fracCopy << abs(E);

    if((E + Bias) <= 0) {/*Check if exp is equal/less than 0*/

      if(number->is_negative == 0) {
         return zero;
      } else {
         return neg_zero;
      }
    }

    result = (result | fracSet(fracCopy));
    result = (result | expSet((E+Bias)));
    
    if(number->is_negative == 1) {
      result = (result | Sign);
    }

    return result;

  }

    /*Main body of the function after all other cases have been dealt with*/
    if (number->whole != 1) {
       E = wholeShifter(wholeCopy, fracCopy);
    }
    
    
    fracCopy = wholeShifter2(wholeCopy, fracCopy);
    fracCopy = fracRounder(fracCopy);


    if(E + Bias >= 15) {/*Infinity check*/
      if (number->is_negative == 0) {
        return pos_infinity;
      } else {
        return neg_infinity;
      }
      
    }

    result = (result | fracSet(fracCopy));
    result = (result | expSet((E+Bias)));

    if (number->is_negative == 1) {
      result = (result | Sign);
      return result;
    } else {
      return result;
    }
      
}

/*Helper function to get exp and convert it to E by checking through bits 8-11*/
int expCountToE(tinysf_s value) {
  int exp, E;
  int expBit = (1 << 8);
  exp = 0;

  if((expBit & value) != 0) {/*Check bit 8*/
    exp = exp + 1;
  }

  expBit = (1 << 9);

  if((expBit & value) != 0) {/*Check bit 9*/
    exp = exp + 2;
  }

  expBit = (1 << 10);

  if((expBit & value) != 0) {/*Check bit 10*/
    exp = exp + 4;
  }

  expBit = (1 << 11);
  
  if((expBit & value) != 0) {/*Check bit 11*/
    exp = exp + 8;
  }

  E = (exp - Bias);
  return E;
  

}

/* toNumber - Converts a TINYSF Value into a Number Struct (whole and fraction parts)
 *  - number is managed by zuon, DO NOT FREE or re-Allocate number.
 *    - (It is already allocated)
 *  - Follow the project documentation for this function.
 * Return 0.
 */

int toNumber(Number_s *number, tinysf_s value) {

  int E, temp;

  if(value == 0x00000f00) {/*Infinity check*/
    number->is_infinity = 1;
    return 0;
  }

  if(value == 0x00001f00) {/*Infinity check*/
    number->is_infinity = 1;
    number->is_negative = 1;
    return 0;
  }

  if(((value & 0x00000f00) == 0x00000f00) && ((value & 0x00000ff) != 0)) {/*NaN check*/
    number->is_nan = 1;
    return 0;
  }



  if((value & 0x00000f00) == 0) {/*Zero Check*/
    number->whole = 0;
    number->fraction = 0;
    return 0;
  }

  E = expCountToE(value);

  if(E == 0) {/*Case if decimal is in right place already*/
    number->fraction = value << 24;
    number->whole = 1;

    if((value & 0x00001000) != 0) {
      number->is_negative = 1;
    }

    return 0;
  }


  if(E < 0)  {/*Shift once and apply the whole number then shift the frac field based on the remaining shifts (abs(E) - 1)*/

    number->fraction = value << 24;
  
    number->whole = 0;

    number->fraction = number->fraction >> 1;

    number->fraction = number->fraction | firstFrac;

    number->fraction = number->fraction >> (abs(E) - 1);

    if((value & Sign) != 0) {
      number->is_negative = 1;
    }

    return 0;

  }
  /*Main body of function when all cases are checked*/
  number->fraction = (value << 24);
  number->whole = 1;
  while(E > 0) {

    number->whole = (number->whole << 1);

    if((number->fraction & 0x80000000) != 0) {
      number->whole = (number->whole | 1);
    }

    number->fraction = (number->fraction << 1);

    E--;

  }

  if((value & Sign) != 0) {
      number->is_negative = 1;
    }
  
  return 0; // Replace this Line with your Code!
}

/*Helper function to use incase a value has a 0*/
tinysf_s zeroCheck(char operator, tinysf_s val1, tinysf_s val2) {

  if(operator == '+') {

    if((val1 == neg_zero) && (val2 == neg_zero)) {/*-0-0*/
      return neg_zero;
    }

    if((val1 == 0) && (val2 == 0)) {/*0+0*/
      return 0;
    }

    if((val1 == 0) && (val2 == neg_zero)){/*0 - 0*/
      return 0;
    }

    if((val1 == neg_zero) && (val2 == 0)) {/* - 0 + 0*/
      return 0;
    }

    if((val1 == 0) || (val1 == neg_zero)) {/*0 + X*/
      return val2;
    }

    if((val2 == 0) || (val2 == neg_zero)) {/*X + 0*/
      return val1;
    }
  }

  if(operator == '-') {

    if((val1 == neg_zero) && (val2 == 0)) {
      return neg_zero;
    }

    if((val1 == 0) && ((val2 == 0) || (val2 == neg_zero))){
      return 0;
    }

    if((val1 == neg_zero) && (val2 == neg_zero)) {
      return 0;
    }

    if((val1 == 0) || (val1 == neg_zero)) {
      return (val2 | Sign);
    }

    if((val2 == 0) || (val2 == neg_zero)) {
      return val1;
    }


  }

  if(operator == '*') {
      return 0;
    }
  
  return 0;

}
/*Helper function to get the number as a whole*/
unsigned int toWhole(tinysf_s value) {
  
  unsigned int Mantissa;
  Mantissa = 1;
  value = (value & 0x000000ff); /*Clear all bits but the frac field*/

  while((value & 0x000000ff) != 0) {
    if((value & (1 << 7)) != 0) {
      value = (value << 1);
      Mantissa = (Mantissa << 1);
      Mantissa = (Mantissa | 1);
    } else {
    value = (value << 1);
    Mantissa = (Mantissa << 1);
    } 
  }

  return Mantissa;


}

/*Helper function to get the E of a number to as if it was entirely whole with no frac*/
int shiftsForWhole(tinysf_s value) {

  int E;
  E = 0;
  
  value = (value & 0x000000ff);

  while((value & 0x000000ff) != 0) {
      value = value << 1;
      E = E - 1;
  }

  return E;


}
tinysf_s subtraction(tinysf_s val1, tinysf_s val2) {
  int E1, E2, E;
  unsigned int M1, M2, M, frac, exp, result;
  int S1, S2, S;
  
  if((val1 & Sign) == 0) {/*See which value is negative/positive*/
    S1 = 0;
  } else {
    S1 = 1;
  }

  if((val2 & Sign) == 0) {
    S2 = 0;
  } else {
    S2 = 1;
  }

  frac = 0;
  exp = 0;
  result = 0;

  val1 = val1 & ~(Sign); /*Clear sign to avoid errors*/
  val2 = val2 & ~(Sign);

  E1 = expCountToE(val1);
  E2 = expCountToE(val2);
  E1 = E1 + shiftsForWhole(val1);
  E2 = E2 + shiftsForWhole(val2);
  M1 = toWhole(val1);
  M2 = toWhole(val2);

  if(E1 < E2) {/*Make E's equal*/
    
    M2 = (M2 << (abs(abs(E2) - abs(E1))));
    E = E1;

  }

  if(E1 > E2) {/*Make E's Equal*/

    M1 = (M1 << (abs(abs(E1) - abs(E2))));
    E = E2;

  }
  if(E1 == E2) {
    E = E1;
  }

  if((S1 == 1) && (S2 == 0)) {/*-M1 - M2*/

      M = M1 + M2;
      S = 1;

  }

  if((S1 == 1) && (S2 == 1)) {/*-M1 + M2*/
    
    if(M1 > M2) {
      M = M1 - M2;
      S = 1;
    } else {
      M = M2 - M1;
      S = 0;
    }
  }

  if((S1 == 0) && (S2 == 1)) {/*M1 + M2*/
    M = M1 + M2;
    S = 0;
  }


  if((S1 == 0) && (S2 == 0)) {/*M1 - M2*/

    if(M1 > M2) {
      M = M1 - M2;
      S = 0;
    } else {
      M = M2 - M1;
      S = 1;
    }
  }

  frac = wholeShifter2(M, frac);
  frac = fracRounder(frac);

  while(M != 1) {
    M = (M >> 1);
    E++;
  }

  exp = E + Bias;

  if(exp <= 0) {
    return 0;
  }

  if(exp >= 15) {
    return pos_infinity;
  }

  result = (result | fracSet(frac));
  result = (result | expSet(exp));

  if(S == 1) {
    result = (result | Sign);
  }

  return result;

}

/*Helper function for addition*/
tinysf_s addition(tinysf_s val1, tinysf_s val2) {
  int E1, E2, E;
  unsigned int M1, M2, M, frac, exp, result;
  int S1, S2, S;
  
  if((val1 & Sign) == 0) {/*See which value is negative/positive*/
    S1 = 0;
  } else {
    S1 = 1;
  }

  if((val2 & Sign) == 0) {
    S2 = 0;
  } else {
    S2 = 1;
  }

  frac = 0;
  exp = 0;
  result = 0;

  val1 = val1 & ~(Sign); /*Clear sign to avoid errors*/
  val2 = val2 & ~(Sign);

  E1 = expCountToE(val1);
  E2 = expCountToE(val2);
  E1 = E1 + shiftsForWhole(val1);
  E2 = E2 + shiftsForWhole(val2);
  M1 = toWhole(val1);
  M2 = toWhole(val2);

  if(E1 < E2) {/*Make E's equal*/
    
    M2 = (M2 << (abs(abs(E2) - abs(E1))));
    E = E1;

  }

  if(E1 > E2) {/*Make E's Equal*/

    M1 = (M1 << (abs(abs(E1) - abs(E2))));
    E = E2;

  }
  if(E1 == E2) {
    E = E1;
  }

  if((S1 == 1) && (S2 == 0)) {

    if(M1 > M2) {
      M = M1 - M2;
      S = 1;
    } else {
      M = M2 - M1;
      S = 0;
    }

  }

  if((S1 == 1) && (S2 == 1)) {
    
    M = M1 + M2;
    S = 1;
    
  }

  if((S1 == 0) && (S2 == 1)) {
    
    if(M2 > M1) {
      M = M2 - M1;
      S = 1;
    } else {
      M = M1 - M2;
      S = 0;
    }
  }

  if((S1 == 0) && (S2 == 0)) {
    M = M1 + M2;
  }

  frac = wholeShifter2(M, frac);
  frac = fracRounder(frac);

  while(M != 1) {
    M = (M >> 1);
    E++;
  }

  exp = E + Bias;

  if(exp <= 0) {
    return 0;
  }

  if(exp >= 15) {
    return pos_infinity;
  }

  result = (result | fracSet(frac));
  result = (result | expSet(exp));

  if(S == 1) {
    result = (result | Sign);
  }

  return result;

}

/*Helper function to multiply to numbers*/
tinysf_s multiplication(tinysf_s val1, tinysf_s val2) {
  int E1, E2, E;
  unsigned int M1, M2, M, frac, exp, result;
  int S1, S2;
  frac = 0;
  exp = 0;
  result = 0;

  S1 = val1 & Sign;
  S2 = val2 & Sign;

  val1 = val1 & ~(Sign); /*Clear sign to avoid errors*/
  val2 = val2 & ~(Sign);

  E1 = expCountToE(val1);
  E2 = expCountToE(val2);
  E1 = E1 + shiftsForWhole(val1);
  E2 = E2 + shiftsForWhole(val2);
  M1 = toWhole(val1);
  M2 = toWhole(val2);

  M = M1 * M2;
  E = E1 + E2;

  frac = wholeShifter2(M, frac);
  frac = fracRounder(frac);

  while(M != 1) {
    M = (M >> 1);
    E++;
  }

  exp = E + Bias;

  if(exp <= 0) {
    if((S1 ^ S2) != 0) {
      return neg_zero;  
    }
    return 0;
  }

  if(exp >= 15) {
    if((S1 ^ S2) != 0) {
      return neg_infinity;  
    }
    return pos_infinity;
  }

  result = (result | fracSet(frac));
  result = (result | expSet(exp));

  if((S1 ^ S2) != 0) {
    result = (result | Sign);
  }

  return result;
}


/*Helper Function to check the special cases within the operator function*/
tinysf_s specialCasesCheck(char operator, tinysf_s val1, tinysf_s val2) {
  
  /*Addition check*/
  if(operator == '+') {
    
    if((val1 == pos_infinity) && (val2 == pos_infinity)) {/*Infinity + Infinity */
      return pos_infinity;
    }

    if((((val1 & pos_infinity) == pos_infinity) && ((val1 & 0x000000ff) != 0)) || (((val2 & pos_infinity) == pos_infinity) && ((val2 & 0x000000ff) != 0))){/*NaN + Anything*/
      return NaN;
    }

    if(((val1 == pos_infinity) || (val2 == pos_infinity)) && ((val1 == neg_infinity) || (val2 == neg_infinity))) {/*Infinity - Negative Infinity*/
      return NaN;
    }

    if((val1 == pos_infinity) || (val2 == pos_infinity)) {/*Infinity + Anything excluding NaN*/
      return pos_infinity;
    }

    if((val1 == neg_infinity) || (val2 == neg_infinity)) {/*Negative Infinity + Anything excluding NaN*/
      return neg_infinity;
    }
  }

  if(operator == '-') {

    if((val1 == pos_infinity) && (val2 == neg_infinity)) {/*Positive Infinity - Negative Infinity*/
      return pos_infinity;
    }

    if((val1 == neg_infinity) && (val2 == pos_infinity)) {/*Negative Infinity - Positive Infinity*/
      return neg_infinity;
    }

    if((val1 == neg_infinity) && (val2 == neg_infinity)) {/*Negative Infinity - Negative Infinity*/
      return NaN;
    }

    if((val1 == pos_infinity) && (val2 ==pos_infinity)) {/*Positive Infinity - Positive Infinity*/
      return NaN;
    }

    if((((val1 & pos_infinity) == pos_infinity) && ((val1 & 0x000000ff) != 0)) || (((val2 & pos_infinity) == pos_infinity) && ((val2 & 0x000000ff) != 0))){/*NaN - Anything*/
      return NaN;
    }

    if((val1 == pos_infinity) || (val2 == neg_infinity)) {
      return pos_infinity;
    }

    if((val1 == neg_infinity) || (val2 == pos_infinity)) {
      return neg_infinity;
    }
  }

  if(operator == '*') {
    
    if((val1 == pos_infinity) && (val2 == pos_infinity)) {/*Infinity * Infinity*/
      return pos_infinity;
    }

    if((val1 == neg_infinity) && (val2 == neg_infinity)) {/*Negative Infinity * Negative Infinity*/
      return pos_infinity;
    }

    if((val1 == neg_infinity) && (val2 == pos_infinity)) {/* Negative Infinity * Infinity*/
      return neg_infinity;
    }

    if((val1 == pos_infinity) && (val2 == neg_infinity)) {/*Infinity * Negative Infinity*/
      return neg_infinity;
    }


    if((((val1 & pos_infinity) == pos_infinity) && ((val1 & 0x000000ff) != 0)) || (((val2 & pos_infinity) == pos_infinity) && ((val2 & 0x000000ff) != 0))){/*NaN * Anything*/
      return NaN;
    }

    if((((val1 == 0) || (val1 == neg_zero)) || (val2 == 0 || val2 == neg_zero)) && (((val1 & pos_infinity) == pos_infinity) || ((val2 & pos_infinity) == pos_infinity))) {
      return NaN;
    }

  }

  return 0;/*Return 0 on success*/
}

/* opTinySF - Performs an operation on two tinySF values
 *  - Follow the project documentation for this function.
 * Return the resulting tinysf_s value or -1 if operator was invalid.
 */
tinysf_s opTinySF(char operator, tinysf_s val1, tinysf_s val2) {

  if((operator != '+') && (operator != '-') && (operator != '*')) {/*Operator Check*/
    return -1;
  }

  tinysf_s result;

  result = specialCasesCheck(operator, val1, val2);

  if(result != 0) {
    return result;
  }

  if((val1 == 0) || (val1 == neg_zero) || (val2 == 0) || (val2 == neg_zero)) {
    result = zeroCheck(operator, val1, val2);
  }

  if((val1 == val2) && (operator == '-')) {/*Identical Check*/
    result = 0;
    return result;
  }

  if (operator == '*') {
    result = multiplication(val1, val2);
    return result;
  }

  if(operator == '+') {
    result = addition(val1, val2);
    return result;
  }

  if(operator == '-') {
    result = subtraction(val1, val2);
    return result;
  }






  return -1; // Replace this Line with your Code!
}





/* negateTINYSF - Negates a TINYSF Value.
 *  - Follow the project documentation for this function.
 * Return the resulting TINYSF Value
 */
tinysf_s negateTinySF(tinysf_s value) {

  value = value ^ Sign;

  return value; // Replace this Line with your Code!
}
