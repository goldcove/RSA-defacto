/*******************************************************************************
* RSA defacto'
* RSA public key (semi prime) factoring algorithm
*
* A somewhat dirty C implementation of the 'RSA defacto' tecnique.  
* 
* Copyright 2016 Goldcove a.k.a. 2f426c5f72137f8ba47eb4db48c475c98ee5ba82de1f555b7902d2c5975fa2a7 (sha256). 
* This work is licensed under a GPLv3 or later license.
* 
* Incorporates gmp for bignumber support
* Added simple interupt handler to print status
* Only checking odd numbers (RSA brute force)
*
* TODO: Fix indent in "loop"
* TODO: Make Create inital X better to handle very large numbers
*
* Compile: gcc filename.c -lgmp [-O3]
* 
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "gmp.h"

_Bool printStatus = 0;
_Bool run = 1;
time_t starttime, stoptime;
unsigned long int cnt = 0;
int interval = 20; //Prints x shifts every n second

void intHandler (int sig) { // stops the execution
  run = 0;  
}

void intUsrHandler (int sig) { // Prints progress
  printStatus = 1;  
}

int main (int argc, char *argv[]) {
  signal(2, intHandler); //SIGINT
  signal(10, intUsrHandler); //SIGUSR1
  signal(15, intHandler); //SIGTERM
  
  printf("DEBUG: build from file %s\n", __FILE__);
  printf("DEBUG: ULLONG_MAX %#llx\n", ULLONG_MAX);
  // Test if cli argument given
  if (argc<2) 
    { printf("ERROR: Please supply a number.\nUse 0x or 0X prefix to specify hex number, 0b or 0B for binary, O for octal. Without prefix decimal is assumed.\n"); 
      return 1; 
    }

  //Initialize
  mpf_t BIG_tmp, sqrt_tmp;
  mpz_t x, y, z, shift, tmp, BIG;
  //mpf_set_default_prec (100000);
  mpf_init(BIG_tmp);
  mpf_init(sqrt_tmp);
  mpz_init(x);
  mpz_init(y);
  mpz_init(z);
  mpz_init(tmp); // TODO use 'tmp' on temp use variables...
  mpz_init(shift);
    
  gmp_printf("Number to factor: %s\n", argv[1]);

  //TODO add error handling for mpz mpf initialization
  mpz_init_set_str (BIG, argv[1], 0);  // set BIG from cli argument  if base is unspecified, assume decimal
  mpf_set_z(BIG_tmp, BIG);  // copy to float


  if (argc == 2) {//No resume, do preloop
  

    /* Sanity check of number */

    
    /* PRELOOP */
    printf("DEBUG: Generate sqrt\n"); //DEBUG
    mpf_sqrt(sqrt_tmp, BIG_tmp);  //generate square root
    printf("DEBUG: Make initial x from sqrt\n"); //DEBUG
    mpz_set_f(x, sqrt_tmp);  // make initial x from sqrt
    printf("DEBUG: Clear BIG_tmp, sqrt_tmp\n");//DEBUG
    mpf_clear (BIG_tmp); // Done with BIG_tmp and sqrt_tmp
    mpf_clear (sqrt_tmp);

    printf("DEBUG: Copy x to y\n");//DEBUG
    mpz_set (y, x); // copy x to y
    printf("DEBUG: Generate sqrt\n");  //DEBUG
    mpz_mul(tmp, x, y); // generate square
    printf("DEBUG: make initial z\n"); //DEBUG
    mpz_sub (z, BIG, tmp);  //z= BIG - (x*y);
    gmp_printf("DEBUG: BIG 0x%Zx\nx 0x%Zx\ny 0x%Zx\nz 0x%Zx\n", BIG, x, y, z); //DEBUG
    if (mpz_cmp(z, x) >= 0) { printf("DEBUG: z larger or equal to x\n"); } //DEBUG
    if (mpz_cmp(z, BIG) >= 0) { printf("DEBUG: z larger or equal to BIG\n"); } //DEBUG
    else { printf("DEBUG: z is less than BIG\n"); } //DEBUG

    /* create starting x */
    printf("DEBUG: Create starting x\n");
    if (mpz_cmp(z, x) >= 0) {
      mpz_tdiv_q(tmp, z, y); //z / y
      mpz_add(x, x, tmp); //x+ mod
      if (mpz_cmp(tmp, BIG) >= 0) { printf("ERROR: tmp is bigger or equal to BIG\n"); } //DEBUG
      mpz_mul(tmp, x, y); // generate small squarempz_mod      
      mpz_sub (z, BIG, tmp);  //z = BIG - (x * y);
      }
    
    //The following while loop should not be needed anymore...
    while (mpz_cmp(z, x) >= 0) {   // z >= x Create starting x y by 
      printf("DEBUG: x+1 loop\n"); //DEBUG
      mpz_add_ui(x, x, 1UL); //x += 1;
      mpz_mul(tmp, x, y); // generate small squarempz_mod
      mpz_sub (z, BIG, tmp);  //z = BIG - (x * y);
    }
    
    if (mpz_cmp(z, x) >= 0) { printf("DEBUG: z larger or equal to x\n"); } //DEBUG
    else { printf("DEBUG: z is less than x\n"); } //DEBUG
    if (mpz_cmp(z, BIG) >= 0) { printf("DEBUG: z larger or equal to BIG\n"); } //DEBUG
    else { printf("DEBUG: z is less than BIG\n"); } //DEBUG
  }  
  else { //resume
    printf("DEBUG: Resume\n");
    if (argc < 5) { printf ("ERROR: Please supply BIG, x, y, z\n"); return(1); }
    mpz_init_set_str (x, argv[2], 0);  // set x from cli argument
    mpz_init_set_str (y, argv[3], 0);  // set y from cli argument
    mpz_init_set_str (z, argv[4], 0);  // set z from cli argument   
    //printf("Resuming: "); //debug
  }
  /* test if y is odd. assuming that we are factoring rsa we only need to test odd factors */
  if ( mpz_even_p(y) ) { //run once to make y odd 
    mpz_add(tmp, x, z); //x+z
    mpz_sub_ui(y, y, 1); 
    mpz_tdiv_qr (shift, z, tmp, y); // extract shift and z from (x+z) / y
    mpz_add(x, x, shift); //x = x + shift;
  }
  /* TODO running check to test if x is odd. assuming that we are factoring rsa we only need to test odd factors */



  if (mpz_cmp_ui(z, 0) == 0) { //z == 0
    gmp_printf( "Factors found! (preloop z==0) FactA 0x%Zx FactB 0x%Zx\n", x, y);
    gmp_printf("BIG number 0x%Zx (FactA * FactB)\n", tmp); //debug
    return(0);
  }
  /* TEMP comment out for troubleshoot
  mpz_clear (BIG); // Done with BIG
  */
  if (mpz_cmp(z, y) == 0) { //(z == y)
    mpz_add_ui(x, x, 1UL); //x += 1;
    mpz_mul(tmp, x, y); // generate small square
    gmp_printf( "Factors found! (preloop z==y) FactA 0x%Zx FactB 0x%Zx\n", x, y);
    gmp_printf("BIG number 0x%Zx (FactA * FactB)\n", tmp); //debug
    return(0);
  }
 
  gmp_printf("INFO: x 0x%Zx y 0x%Zx z 0x%Zx\n", x, y, z); //debug
  printf("Starting factoring loop. Have some coffee and wait, this might take a while...\n"); //debug  
  /* loop */
  while (run)  { //skip test of y since y=1 will always return a "good" factor, ie end the program. (mpz_cmp_si(y, 1) > 0) //(y > 1) 
    if (cnt == 0) { time(&starttime); /*get epoch in sec*/ }
    
    //mpz_add_ui(x, x, 1); // x + 1 TODO optimize?
    mpz_add(tmp, x, z); //x+ x+z
    mpz_add(tmp, tmp, x); 

      mpz_sub_ui(y, y, 1); //y-1
      mpz_sub_ui(y, y, 1); //y-1          
      mpz_tdiv_qr (shift, z, tmp, y); // extract shift and z from (x+z) / y
    
    mpz_add(x, x, shift); //x = x + shift;
    cnt += mpz_get_ui(shift); //Add shift to count, convert from mpz_t to unsigned long int

    mpz_mul(tmp, x, y);
    mpz_add(tmp, tmp, z);
    if (mpz_cmp(tmp, BIG) != 0) { 
      printf("ERROR: x * y + z differs from BIG. Line %d\n", __LINE__);
      gmp_printf("ERROR: x %Zx y %Zx z %Zx\n", x, y, z);
      return 1;        
      }
  
  
    if (mpz_cmp_ui(z, 0) == 0 ) { //z == 0 //had to remove zULL from test: || zULL == 0
      gmp_printf( "Factors found! (loop z==0) FactA 0x%Zx FactB 0x%Zx line %d\n", x, y, __LINE__); //debug 
      mpz_mul(tmp, x, y); //debug
      gmp_printf("BIG number 0x%Zx (FactA * FactB)\n", tmp); //debug     
      //Logic check of BIG and calculated big
      if (mpz_cmp(BIG,tmp) != 0) { printf("ERROR: LOGIC CHECK FAILED: BIG differs from calculated BIG!\n"); return(1);} //This should not happen...
      else { return(0); }
    }  
    if (printStatus) { //  || !cnt print status if sigusr or cnt flops over to 0 (unsigned int)
      gmp_printf("x 0x%Zx y 0x%Zx z 0x%Zx shift 0x%Zx\n", x, y, z, shift); 
      printStatus = 0;
    }

    /*Print progress counter
      loop per second*/ 
    time(&stoptime);

    if (((int) stoptime) > (((int)starttime) + interval) ) {
      // TODO: I would like to use \r but for some reason it does not work???
      printf("INFO: X shifts per second 0x%lx ", cnt / ((unsigned long int) (stoptime-starttime)) );
      gmp_printf("Y: 0x%Zx\n", y);
      cnt = 0;
    }
  }  /*END LOOP*/

  if (!run) { 
    gmp_printf("\nABORTING! x 0x%Zx y 0x%Zx z 0x%Zx\n", x, y, z); 
  }
  else { // this should never run...
    gmp_printf("Factor NOT found. Debug x 0x%Zx y 0x%Zx\n",x,y); 
  }
  return(1);
}
