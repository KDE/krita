/*
    $Id$

    kCalculator, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no
    
    The stack engine conatined in this file was take from 
    Martin Bartlett's xfrmcalc
   
    portions:	Copyright (C) 1996 Bernd Johannes Wuebben   
                                   wuebben@math.cornell.edu

    portions: 	Copyright (C) 1995 Martin Bartlett
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/


#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <kapp.h>
#include <klocale.h>
#include "kcalc.h"

// Undefine HAVE_LONG_DOUBLE for Beta 4 since RedHat 5.0 comes with a borken
// glibc

#ifdef HAVE_LONG_DOUBLE
#undef HAVE_LONG_DOUBLE
#endif

extern QList<CALCAMNT> temp_stack; 
last_input_type last_input;
char		display_str[DSP_SIZE+1];

stack_ptr	top_of_stack = NULL;	
stack_ptr	top_type_stack[2] = { NULL, NULL };	
int 		stack_next, stack_last;
stack_item	process_stack[STACK_SIZE];

int    		inverse = FALSE;
int   	 	precedence_base = 0;
num_base    	current_base = NB_DECIMAL;
int    		input_limit = 0;
int    		input_count = 0;

//item_contents 	display_data = { ITEM_AMOUNT, 0 };
item_contents 	display_data;

int 		display_size = DEC_SIZE;
 
int    		hyp_mode = 0;
int    		angle_mode = ANG_DEGREE;
int		refresh_display; // if 1 we start a new number 
int		display_error = 0;
int		decimal_point = 0;
int		percent_mode = 0;
bool 		eestate = false; // if true then we are in ee input mode
    
CALCAMNT 	pi;
CALCAMNT	memory_num = 0.0;

int precedence[14] = { 0, 1, 2, 3, 4, 4, 5, 5, 6, 6, 6, 7, 7, 6 };

    int		adjust_op[14][3] = {
				{FUNC_NULL,     FUNC_NULL,     FUNC_NULL},
				{FUNC_OR,       FUNC_OR,       FUNC_XOR },
				{FUNC_XOR,      FUNC_XOR,      FUNC_XOR },
				{FUNC_AND,      FUNC_AND,      FUNC_AND },
				{FUNC_LSH,      FUNC_LSH,      FUNC_RSH },
				{FUNC_RSH,      FUNC_RSH,      FUNC_RSH },
				{FUNC_ADD,      FUNC_ADD,      FUNC_ADD },
				{FUNC_SUBTRACT, FUNC_SUBTRACT, FUNC_SUBTRACT},
				{FUNC_MULTIPLY, FUNC_MULTIPLY, FUNC_MULTIPLY},
				{FUNC_DIVIDE,   FUNC_DIVIDE,   FUNC_DIVIDE},
				{FUNC_MOD,      FUNC_MOD,      FUNC_INTDIV },
				{FUNC_POWER,    FUNC_POWER,    FUNC_PWR_ROOT},
				{FUNC_PWR_ROOT, FUNC_PWR_ROOT, FUNC_PWR_ROOT},
				{FUNC_INTDIV,   FUNC_INTDIV,   FUNC_INTDIV},
    };
/*
    char		*function_desc[] = {

      "Null",
      "Or",
      "Exclusive Or",
      "And",
      "Left Shift",
      "Right Shift",
      "Add",
      "Subtract",
      "Multiply",
      "Divide",
      "Modulus"
      "Power"
      "Reciprocal Power"
      "Integer Division"
    };
    */    
    Arith      	Arith_ops[14] = { NULL,
				  ExecOr, 
				  ExecXor,  	
				  ExecAnd,  
				  ExecLsh,  
				  ExecRsh,  
				  ExecAdd, ExecSubtract, 
				  ExecMultiply, 
				  ExecDivide, ExecMod,
				  ExecPower, ExecPwrRoot, 
				  ExecIntDiv
    };	

    Prcnt     	Prcnt_ops[14] = { NULL,
				  NULL, 
				  NULL,
				  NULL,
				  NULL,
				  NULL,
				  ExecAddSubP,  ExecAddSubP,  
				  ExecMultiplyP, 
				  ExecDivideP, ExecDivideP,
				  ExecPowerP, ExecPwrRootP, 
				  ExecDivideP
    };	


void QtCalculator::InitializeCalculator(void) {
 
  //
  // Basic initialization involves initializing the calcultion
  // stack, forcing the display to refresh to zero, and setting
  // up the floating point excetion signal handler to trap the
  // errors that the code can/has not been written to trap.
  //
  // We also calculate pi as double the arc sine of 1. 
  //

  display_data.s_item_type = ITEM_AMOUNT;
  display_data.s_item_data.item_amount = 0.0;
  display_data.s_item_data.item_func_data.item_function = 0;
  display_data.s_item_data.item_func_data.item_precedence = 0;
  
  void fpe_handler(int fpe_parm);
  struct sigaction  fpe_trap;

  fpe_trap.sa_handler = &fpe_handler;
#ifdef SA_RESTART
  fpe_trap.sa_flags = SA_RESTART;
#endif

  sigaction(SIGFPE, &fpe_trap, NULL);
  
  RefreshCalculator();
  pi = ASIN(1L) * 2L;
}		

void fpe_handler(int fpe_parm)
{
  (void) fpe_parm;

  display_error = 1;
  DISPLAY_AMOUNT = 0L;

}		

void QtCalculator::setData( const QRect& _range, const char *_table )
{
  table_range = _range;
  table_name = _table;
}

void QtCalculator::setValue( double _value )
{
  last_input = DIGIT;
  DISPLAY_AMOUNT = _value;
  decimal_point = 0;
  refresh_display = 1;
  input_count = 0;

  UpdateDisplay();
}

void QtCalculator::setLabel( const char *_text )
{
  last_input = DIGIT;
  DISPLAY_AMOUNT = 0L;
  decimal_point = 0;
  refresh_display = 0;
  input_count = 0;

  calc_display->setText( _text );
}

void QtCalculator::RefreshCalculator(void)
{
	InitStack();
	display_error = 0;
	DISPLAY_AMOUNT = 0L;
	inverse = FALSE;
	UpdateDisplay();
	last_input = DIGIT; // must set last to DIGIT after Update Display in order
	                    // not to get a display holding e.g. 0.000  
	input_count = 0;
	decimal_point = 0;
}		

void QtCalculator::EnterDigit(int data)
{

  if(eestate){

    QString string;
    string.setNum(data);
    strcat(display_str,string.data());
    DISPLAY_AMOUNT = (CALCAMNT) strtod(display_str,0);
    UpdateDisplay();
    return;

  }
  
  last_input = DIGIT;
  if (refresh_display) {
    DISPLAY_AMOUNT = 0L;
    decimal_point = 0;
    refresh_display = 0;
    input_count = 0;
  }

  if (!(input_limit && input_count >= input_limit))
    if (DISPLAY_AMOUNT < 0)
      DISPLAY_AMOUNT = decimal_point ? 
	DISPLAY_AMOUNT - ((CALCAMNT)data / 
			  POW(current_base, decimal_point++)) :
    (current_base * DISPLAY_AMOUNT) - data;
    else
      DISPLAY_AMOUNT = decimal_point ? 
	DISPLAY_AMOUNT + ((CALCAMNT)data / 
			  POW(current_base, decimal_point++)) :
    (current_base * DISPLAY_AMOUNT) + data;

  if (decimal_point){
    input_count ++;

#ifdef MYDEBUG
    printf("EnterDigit() inc dec.point:%d\n",input_count);
#endif

  }
  UpdateDisplay();
}

void QtCalculator::button0()
{
  EnterDigit(0);
}

void QtCalculator::button1()
{
  EnterDigit(1);
 }

void QtCalculator::button2()
{
  if (current_base == NB_BINARY)
    return;
  EnterDigit(2);
}

void QtCalculator::button3()
{
  if (current_base == NB_BINARY)
    return;
  EnterDigit(3);
}

void QtCalculator::button4()
{
  if (current_base == NB_BINARY)
    return;
  EnterDigit(4);
}

void QtCalculator::button5()
{
  if (current_base == NB_BINARY)
    return;
  EnterDigit(5);
}

void QtCalculator::button6()
{
  if (current_base == NB_BINARY)
    return;
  EnterDigit(6);
}

void QtCalculator::button7()
{
  if (current_base == NB_BINARY)
    return;
  EnterDigit(7);
}

void QtCalculator::button8()
{
  if ((current_base == NB_BINARY) || (current_base == NB_OCTAL))
    return;
  EnterDigit(8);
}

void QtCalculator::button9()
{
  if ((current_base == NB_BINARY) || (current_base == NB_OCTAL))
    return;
  EnterDigit(9);
}


void QtCalculator::buttonA()
{
  if ((current_base == NB_BINARY) || (current_base == NB_OCTAL)
      || (current_base == NB_DECIMAL))
    return;
  EnterDigit(10);
}


void QtCalculator::buttonB()
{
  if ((current_base == NB_BINARY) || (current_base == NB_OCTAL)
      || (current_base == NB_DECIMAL)) 
    return;
  EnterDigit(11);
}


void QtCalculator::buttonC()
{
   if ((current_base == NB_BINARY) || (current_base == NB_OCTAL)
      || (current_base == NB_DECIMAL))
    return;
  EnterDigit(12);
}


void QtCalculator::buttonD()
{
   if ((current_base == NB_BINARY) || (current_base == NB_OCTAL)
      || (current_base == NB_DECIMAL))
    return;
  EnterDigit(13);
}


void QtCalculator::buttonE()
{
   if ((current_base == NB_BINARY) || (current_base == NB_OCTAL)
      || (current_base == NB_DECIMAL))
    return;
  EnterDigit(14);
}

void QtCalculator::buttonF()
{
   if ((current_base == NB_BINARY) || (current_base == NB_OCTAL)
      || (current_base == NB_DECIMAL))
    return;
  EnterDigit(15);
}



void QtCalculator::EnterDecimal()
{

  if(eestate){
    QApplication::beep();
    return;
  }

  decimal_point = 1;
  if (refresh_display) {
    DISPLAY_AMOUNT = 0L;
    refresh_display = 0;
    input_count = 0;
  }
  
  if (last_input == DIGIT && !strpbrk( display_str,".")){
    
    // if the last input was a DIGIT and we don't have already a period in our 
    // display string then display a period 
    
    calc_display->setText(strcat(display_str, "."));
  }
  else {
    
    // the last input wasn't a DIGIT so we are about to 
    // input a new number in particular we neet do display a "0.".
    
    DISPLAY_AMOUNT = 0L;
    refresh_display = 0;
    //	  decimal_point = 1;
    //	  input_count = 1;
    strcpy(display_str, "0.");
    calc_display->setText(display_str);
  }
}


void QtCalculator::Or()
{
  eestate = false;
  if (inverse){
    EnterStackFunction(2);   // XOR
    inverse = FALSE;
  }
  else {
    EnterStackFunction(1);   // OR
  }
  last_input = OPERATION;
}

void QtCalculator::And()
{
  eestate = false;
  last_input = OPERATION;
  EnterStackFunction(3);
}


void QtCalculator::Shift()
{
  eestate = false;
  last_input = OPERATION; 
  if (inverse){
    EnterStackFunction(5);   // Rsh
    inverse = FALSE;
  }
  else {
    EnterStackFunction(4);   // Lsh
  }
 
}

void QtCalculator::Plus()
{
  eestate = false;
  last_input = OPERATION;
  EnterStackFunction(6);
}

void QtCalculator::Minus()
{
  eestate = false;
  last_input = OPERATION;
  EnterStackFunction(7);

}

void QtCalculator::Multiply()
{
  eestate = false;
  last_input = OPERATION;
  EnterStackFunction(8);
}

void QtCalculator::Divide()
{
  eestate = false;
  last_input = OPERATION;
  EnterStackFunction(9);
}

void QtCalculator::Mod()
{
  eestate = false;
  last_input = OPERATION;
  if (inverse){
    EnterStackFunction(13);   // InvMod
    inverse = FALSE;
  }
  else {
    EnterStackFunction(10);   // Mod
  }
}

void QtCalculator::Power()
{
  eestate = false;
  last_input = OPERATION;  
  if (inverse){
    EnterStackFunction(12);   // InvPower
    inverse = FALSE;
  }
  else {
    EnterStackFunction(11);   // Power
  }
  
}



void QtCalculator::EnterStackFunction(int data)
{
	item_contents 	new_item;
	int		new_precedence;
	int 		dummy;

	dummy = 0;

	/*
	if (inverse ) {
	  dummy = 3;
	  inverse = FALSE;
	}
	else {
	  dummy = 1;
	}
	*/

	//	printf("data %d dummy %d\n",data,dummy);
	data = adjust_op[data][dummy];
	//	printf("data %d \n",data );

	PushStack(&display_data);

	new_item.s_item_type = ITEM_FUNCTION;
	new_item.s_item_data.item_func_data.item_function = data;
	new_item.s_item_data.item_func_data.item_precedence =
		new_precedence = precedence[data] + precedence_base;

	refresh_display = 1;
	if (UpdateStack(new_precedence))
		UpdateDisplay();
	PushStack(&new_item);
}

void QtCalculator::EnterNegate()
{

  if(eestate){
    QString string;
    string = display_str;
    int pos;
    pos = string.findRev('e',-1,false);
    if(pos == -1)
      return;

    if(display_str[pos+1] == '+')
      display_str[pos+1] = '-';
    else{
      if(display_str[pos+1] == '-')
	display_str[pos+1] = '+';
      else{
	string.insert(pos +1,'-');
	strncpy(display_str,string.data(),DSP_SIZE);
      }
    }
    DISPLAY_AMOUNT = (CALCAMNT)strtod(display_str,0);
    UpdateDisplay();
  }
  else{
    //    last_input = OPERATION;
    if (DISPLAY_AMOUNT != 0) {
      DISPLAY_AMOUNT *= -1;
      UpdateDisplay();
    }
  }
  last_input = DIGIT;
}

void QtCalculator::EnterOpenParen()
{
  eestate = false;
	last_input = OPERATION;
	precedence_base += PRECEDENCE_INCR;
	refresh_display = 1;

}

void QtCalculator::EnterCloseParen()
{
  eestate = false;
	last_input = OPERATION;
	PushStack(&display_data);
	refresh_display = 1;
	if (UpdateStack(precedence_base))
		UpdateDisplay();
	if ((precedence_base -= PRECEDENCE_INCR) < 0)
		precedence_base = 0;

}

void QtCalculator::EnterRecip()
{
  eestate = false;
  last_input = OPERATION;
  DISPLAY_AMOUNT = 1 / DISPLAY_AMOUNT;	
  refresh_display = 1;
  UpdateDisplay();
}

void QtCalculator::EnterInt()
{
  eestate = false;
	CALCAMNT work_amount1, work_amount2;

	last_input = OPERATION;	
	if (!inverse){
	  work_amount2 = MODF(DISPLAY_AMOUNT, &work_amount1);
	  DISPLAY_AMOUNT = work_amount2 ;
	    }
	else {
	  DISPLAY_AMOUNT = work_amount1;
	  inverse = FALSE;
	}
	
	refresh_display = 1;
	UpdateDisplay();

}

void QtCalculator::EnterFactorial()
{
  eestate = false;
	CALCAMNT work_amount1, work_amount2;
	int	 incr;

	MODF(DISPLAY_AMOUNT, &work_amount1);

	incr = work_amount1 < 0 ? -1 : 1;
	work_amount2 = work_amount1 - incr;
	while (work_amount1 != 0 && work_amount2 != 0 && !display_error) {
		work_amount1 *= work_amount2;
		work_amount2 -= incr;
		if(isinf(work_amount1)) {
		  display_error=1;
		   break;
		}
	}

	if( work_amount1 == 0.0)
	  work_amount1 = 1.0;

	DISPLAY_AMOUNT = work_amount1;
	refresh_display = 1;
	last_input = OPERATION;
	UpdateDisplay();

}

void QtCalculator::EnterSquare()
{
  eestate = false;
	if (!inverse){
		DISPLAY_AMOUNT *= DISPLAY_AMOUNT;	
	} 
	else if (DISPLAY_AMOUNT < 0)
	display_error = 1;
	else
		DISPLAY_AMOUNT = SQRT(DISPLAY_AMOUNT);	
	refresh_display = 1;
	inverse = FALSE;
	last_input = OPERATION;	
	UpdateDisplay();

}

void QtCalculator::EnterNotCmp()
{
  eestate = false;
	CALCAMNT	boh_work_d;
	long 		boh_work;

	MODF(DISPLAY_AMOUNT, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX)
		display_error = 1;    
		else {
		boh_work = (long int) boh_work_d;
		DISPLAY_AMOUNT = ~boh_work;	
			} 
	refresh_display = 1;
	last_input = OPERATION;
	UpdateDisplay();

}

void QtCalculator::EnterHyp()
{
  
  switch(kcalcdefaults.style){
  case 2:
  case 1:{
    if ( !table_name.isEmpty() )
      useData();
    
    if(!inverse){
    eestate = false; // terminate ee input mode
    DISPLAY_AMOUNT =  stats.count();
    last_input = OPERATION;
    refresh_display = 1;
    UpdateDisplay();
    }
    else{
    inverse = false;
    eestate = false; // terminate ee input mode
    DISPLAY_AMOUNT =  stats.sum();
    last_input = OPERATION;
    refresh_display = 1;
    UpdateDisplay();
    }

    break;
  }

  case 0: {
    // toggle between hyperbolic and standart trig functions
    hyp_mode = !hyp_mode;
  
    if (hyp_mode){
      statusHYPLabel->setText("HYP");
    }
    else{
      statusHYPLabel->setText("");
    }
    break;
  }
  }
}


void QtCalculator::ExecSin(){

  switch(kcalcdefaults.style){

  case 0:{ // trig mode

    ComputeSin();
    break;
  }
  
  case 1:{ // stats mode
    if ( !table_name.isEmpty() )
      useData();
  
    ComputeMean();
    break;
  }

  case 2:{ // table mode
    if ( !table_name.isEmpty() )
      useData();
  
    ComputeMin();
    break;
  }

  }

}

void QtCalculator::ComputeSum()
{
  inverse = false;
  eestate = false;
  DISPLAY_AMOUNT = stats.sum();
  if (stats.error())
      display_error = 1;
  
  refresh_display = 1;
  last_input = OPERATION;
  UpdateDisplay();
}

void QtCalculator::ComputeMul()
{
  inverse = false;
  eestate = false;
  DISPLAY_AMOUNT = stats.mul();
  if (stats.error())
      display_error = 1;
  
  refresh_display = 1;
  last_input = OPERATION;
  UpdateDisplay();
}

void QtCalculator::ComputeMin()
{
  inverse = false;
  eestate = false;
  DISPLAY_AMOUNT = stats.min();
  if (stats.error())
      display_error = 1;
  
  refresh_display = 1;
  last_input = OPERATION;
  UpdateDisplay();
}

void QtCalculator::ComputeMax()
{
  inverse = false;
  eestate = false;
  DISPLAY_AMOUNT = stats.max();
  if (stats.error())
      display_error = 1;
  
  refresh_display = 1;
  last_input = OPERATION;
  UpdateDisplay();
}

void QtCalculator::ComputeMean(){

  if(!inverse){
    eestate = false;
    DISPLAY_AMOUNT = stats.mean();
    if (stats.error())
      display_error = 1;

    refresh_display = 1;
    last_input = OPERATION;
    UpdateDisplay();
  }
  else{
    inverse = false;
    eestate = false;
    DISPLAY_AMOUNT = stats.sum_of_squares();
    if (stats.error())
      display_error = 1;
    refresh_display = 1;
    last_input = OPERATION;
    UpdateDisplay();

  }

}

void QtCalculator::ComputeSin()
{
  CALCAMNT	work_amount;
  eestate = false;
  work_amount = DISPLAY_AMOUNT;

  if (hyp_mode){
    // sinh or arcsinh
    if (!inverse){
      DISPLAY_AMOUNT = SINH( work_amount);
    }
    else {
      DISPLAY_AMOUNT = ASINH( work_amount);
      if (errno == EDOM || errno == ERANGE)
	display_error = 1;
      inverse = FALSE;       // reset the inverse flag
    }
  }
  else {  
    // sine or arcsine
    if (!inverse){
      // sine 
      switch (angle_mode) {
      case ANG_DEGREE:
	work_amount = DEG2RAD(DISPLAY_AMOUNT);
	break;
      case ANG_GRADIENT:
	work_amount = GRA2RAD(DISPLAY_AMOUNT);
	break;
      case ANG_RADIAN:
	work_amount = DISPLAY_AMOUNT;
	break;
      }
      DISPLAY_AMOUNT = SIN( work_amount);
    }
    else {
      // arcsine
      DISPLAY_AMOUNT = ASIN(work_amount);
            switch (angle_mode) {
      case ANG_DEGREE:
	work_amount = RAD2DEG(DISPLAY_AMOUNT);
	break;
      case ANG_GRADIENT:
	work_amount = RAD2GRA(DISPLAY_AMOUNT);
	break;
      case ANG_RADIAN:
	work_amount = DISPLAY_AMOUNT;
	break;
      }
      DISPLAY_AMOUNT = work_amount;
      if (errno == EDOM || errno == ERANGE)
	display_error = 1;
      inverse = FALSE; 		// reset the inverse flag
    }
  }
 
// Now a cheat to help the weird case of COS 90 degrees not being 0!!!
  
  if (DISPLAY_AMOUNT < POS_ZERO && DISPLAY_AMOUNT > NEG_ZERO)
    DISPLAY_AMOUNT=0;
  refresh_display = 1;
  last_input = OPERATION;
  UpdateDisplay();

}

void QtCalculator::ExecCos(){

  switch(kcalcdefaults.style){

  case 0:{ // trig mode

    ComputeCos();
    break;
  }
  
  case 1:{ // stats mode
      if ( !table_name.isEmpty() )
      useData();

    ComputeStd();
    break;
  }

  case 2:{ // table mode
      if ( !table_name.isEmpty() )
      useData();

    ComputeMax();
    break;
  }

  }

}

void QtCalculator::ComputeStd(){

  if(!inverse){ // std (n-1)
    inverse = false;
    eestate = false;
    DISPLAY_AMOUNT = stats.std();

    if (stats.error()){
      display_error = 1;
    }

    refresh_display = 1;
    last_input = OPERATION;
    UpdateDisplay();
  }
  else{ // std (n)

    inverse = false;
    eestate = false;
    DISPLAY_AMOUNT = stats.sample_std();

    if (stats.error())
      display_error = 1;

    refresh_display = 1;
    last_input = OPERATION;
    UpdateDisplay();



  }

}

void QtCalculator::ComputeCos()
{
  CALCAMNT	work_amount;
  eestate = false;
  work_amount = DISPLAY_AMOUNT;
  
  if (hyp_mode){
    // cosh or arccosh
    if (!inverse){
      DISPLAY_AMOUNT = COSH( work_amount);
    }
    else {
      DISPLAY_AMOUNT = ACOSH( work_amount);
      if (errno == EDOM || errno == ERANGE)
	display_error = 1;
      inverse = FALSE;       // reset the inverse flag
    }
  }
  else {  
    // cosine or arccosine
    if (!inverse){
      // sine 
      switch (angle_mode) {
      case ANG_DEGREE:
	work_amount = DEG2RAD(DISPLAY_AMOUNT);
	break;
      case ANG_GRADIENT:
	work_amount = GRA2RAD(DISPLAY_AMOUNT);
	break;
      case ANG_RADIAN:
	work_amount = DISPLAY_AMOUNT;
	break;
      }
      DISPLAY_AMOUNT = COS( work_amount);
    }
    else {
      // arccosine
      DISPLAY_AMOUNT = ACOS(work_amount);
      switch (angle_mode) {
      case ANG_DEGREE:
	work_amount = RAD2DEG(DISPLAY_AMOUNT);
	break;
      case ANG_GRADIENT:
	work_amount = RAD2GRA(DISPLAY_AMOUNT);
	break;
      case ANG_RADIAN:
	work_amount = DISPLAY_AMOUNT;
	break;
      }
      
      DISPLAY_AMOUNT = work_amount;

      if (errno == EDOM || errno == ERANGE)
	display_error = 1;
      inverse = FALSE; 		// reset the inverse flag
    }
  }
 
// Now a cheat to help the weird case of COS 90 degrees not being 0!!!

  
  if (DISPLAY_AMOUNT < POS_ZERO && DISPLAY_AMOUNT > NEG_ZERO)
    DISPLAY_AMOUNT=0;
  refresh_display = 1;
  last_input = OPERATION;
  UpdateDisplay();

}

void QtCalculator::ExecTan(){

  switch(kcalcdefaults.style){

  case 0:{ // trig mode

    ComputeTan();
    break;
  }
  
  case 2:
  case 1:{ // stats mode
  
    if ( !table_name.isEmpty() )
      useData();

    ComputeMedean();
    break;
  }

  }

}

void QtCalculator::ComputeMedean(){

  if(!inverse){ // std (n-1)
    inverse = false;
    eestate = false;
    DISPLAY_AMOUNT = stats.median();

    if (stats.error()){
      display_error = 1;
    }

    refresh_display = 1;
    last_input = OPERATION;
    UpdateDisplay();
  }
  else{ // std (n)

    inverse = false;
    eestate = false;
    DISPLAY_AMOUNT = stats.median();

    if (stats.error())
      display_error = 1;

    refresh_display = 1;
    last_input = OPERATION;
    UpdateDisplay();

  }
}


void QtCalculator::ComputeTan()
{
  CALCAMNT	work_amount;
  eestate = false;
  work_amount = DISPLAY_AMOUNT;

  if (hyp_mode){
    // tanh or arctanh
    if (!inverse){
      DISPLAY_AMOUNT = TANH( work_amount);
    }
    else {
      DISPLAY_AMOUNT = ATANH( work_amount);
      if (errno == EDOM || errno == ERANGE)
	display_error = 1;
      inverse = FALSE;       // reset the inverse flag
    }
  }
  else {  
    // tan or arctan
    if (!inverse){
      // tan
      switch (angle_mode) {
      case ANG_DEGREE:
	work_amount = DEG2RAD(DISPLAY_AMOUNT);
	break;
      case ANG_GRADIENT:
	work_amount = GRA2RAD(DISPLAY_AMOUNT);
	break;
      case ANG_RADIAN:
	work_amount = DISPLAY_AMOUNT;
	break;
      }
      DISPLAY_AMOUNT = TAN( work_amount);
      if (errno == EDOM || errno == ERANGE)
	display_error = 1;
    }
    else {
      // arctan
      DISPLAY_AMOUNT = ATAN(work_amount);
      switch (angle_mode) {
      case ANG_DEGREE:
	work_amount = RAD2DEG(DISPLAY_AMOUNT);
	break;
      case ANG_GRADIENT:
	work_amount = RAD2GRA(DISPLAY_AMOUNT);
	break;
      case ANG_RADIAN:
	work_amount = DISPLAY_AMOUNT;
	break;
      }
      
      DISPLAY_AMOUNT = work_amount;

      if (errno == EDOM || errno == ERANGE)
	display_error = 1;
      inverse = FALSE; 		// reset the inverse flag
    }
  }
 
// Now a cheat to help the weird case of COS 90 degrees not being 0!!!
  
  if (DISPLAY_AMOUNT < POS_ZERO && DISPLAY_AMOUNT > NEG_ZERO)
    DISPLAY_AMOUNT=0;
  refresh_display = 1;
  last_input = OPERATION;
  UpdateDisplay();

}


void QtCalculator::EnterPercent()
{
  eestate = false;
  last_input = OPERATION;
  percent_mode = 1;
  EnterEqual();
  percent_mode = 0;

}

void QtCalculator::EnterLogr()
{

  switch(kcalcdefaults.style){
  case 2:
    {
      if ( !table_name.isEmpty() )
	useData();

      ComputeSum();
      break;
    }
  case 1:{
    
    if ( !table_name.isEmpty() )
      useData();

    if(!inverse){
      eestate = false; // terminate ee input mode
      stats.enterData(DISPLAY_AMOUNT);
      last_input = OPERATION;
      refresh_display = 1;    
      DISPLAY_AMOUNT = stats.count();
      UpdateDisplay();
    }
    else{
      inverse = false;
      last_input = OPERATION;
      refresh_display = 1;
      stats.clearLast();
      setStatusLabel("Last stat item erased");
      DISPLAY_AMOUNT = stats.count();
      UpdateDisplay();

    }

    break;
  }
  case 0:{

    eestate = false;
    last_input = OPERATION;
    
    if (!inverse) {
      if (DISPLAY_AMOUNT <= 0)
	display_error = 1;
      else
	DISPLAY_AMOUNT = LOG_TEN(DISPLAY_AMOUNT);	
      refresh_display = 1;
      UpdateDisplay();
    } else if (inverse) {
      DISPLAY_AMOUNT = POW(10, DISPLAY_AMOUNT);
      refresh_display = 1;
      inverse = FALSE;
      UpdateDisplay();
    }
    break;
  }
  }
}

void QtCalculator::EnterLogn()
{

  switch(kcalcdefaults.style){
  case 2:{
    
    if ( !table_name.isEmpty() )
      useData();

    ComputeMul();

    break;
  }
  case 1:{
    
    if ( !table_name.isEmpty() )
      useData();

    if(!inverse){

      stats.clearAll();
      setStatusLabel((char*)i18n("Stat Mem cleared"));

    }
    else{
      inverse = false;
      UpdateDisplay();
    }

    break;
  }
  case 0:{
    eestate = false;
    last_input = OPERATION;
    if (!inverse) {
      if (DISPLAY_AMOUNT <= 0)
	display_error = 1;
      else
	DISPLAY_AMOUNT = LOG(DISPLAY_AMOUNT);	
      refresh_display = 1;
      UpdateDisplay();
    } else if (inverse) {
      DISPLAY_AMOUNT = EXP(DISPLAY_AMOUNT);	
      refresh_display = 1;
      inverse =FALSE;
      UpdateDisplay();
    }
    break;
  }
  }

}


void QtCalculator::base_selected(int number){
  
  switch(number){
  case 0:	
    SetHex();
    break;
  case 1:	
    SetDec();
    break;
  case 2:       
    SetOct();
    break;
  case 3:
    SetBin();
    break;
  default: // we shouldn't ever end up here
    SetDec();
    }
}


void QtCalculator::angle_selected(int number){
  
  switch(number){
  case 0:	
    SetDeg();
    break;
  case 1:	
    SetRad();
    break;
  case 2:       
    SetGra();
    break;
  default: // we shouldn't ever end up here
    SetRad();
    }
}

void QtCalculator::SetInverse(){

  inverse = ! inverse;
  if (inverse){
    statusINVLabel->setText("INV");
  }
  else{
    statusINVLabel->setText("NORM");
  }
}


void QtCalculator::SetDeg() {
	angle_mode = ANG_DEGREE;
}

void QtCalculator::SetGra() {
	angle_mode = ANG_GRADIENT;
}

void QtCalculator::SetRad() {
	angle_mode = ANG_RADIAN;

}

void QtCalculator::SetHex() {
	/*
	 * Set Hex Mode
	 */

	current_base = NB_HEX;
	display_size = BOH_SIZE;
	decimal_point = 0;
	input_limit = 8;

	UpdateDisplay();
}

void QtCalculator::SetOct() {
	/*
	 * Set Oct Mode
	 */

	current_base = NB_OCTAL;
	display_size = BOH_SIZE;
	decimal_point = 0;
	input_limit = 11;

	UpdateDisplay();
}

void QtCalculator::SetBin() {
	/*
	 * Set Bin Mode
	 */

	current_base = NB_BINARY;
	display_size = BOH_SIZE;
	decimal_point = 0;
	input_limit = 16;

	UpdateDisplay();
}

void QtCalculator::SetDec() 
{
	/*
	 * Set Dec Mode
	 */

	current_base = NB_DECIMAL;
	display_size = DEC_SIZE;
	input_limit = 0;


	UpdateDisplay();
}


void QtCalculator::EE()
{
  if(inverse){
    DISPLAY_AMOUNT = pi;
    inverse = FALSE;
    UpdateDisplay();
  }
  else{
    if(eestate == true)
      eestate = false;
    else{
      eestate = true;
      strcat(display_str,"e");
    }

    UpdateDisplay();
  }

}

void QtCalculator::MR()
{
  eestate = false;
  last_input = OPERATION;
  DISPLAY_AMOUNT = memory_num;
  refresh_display = 1;
  UpdateDisplay();

}
	
void QtCalculator::Mplusminus()
{

  eestate = false;
  EnterEqual();
  if (!inverse)
    memory_num += DISPLAY_AMOUNT;
  else {
    memory_num -= DISPLAY_AMOUNT;
    inverse = FALSE;
  }
}

void QtCalculator::MC()
{

	memory_num = 0;
	refresh_display = 1;
}

void QtCalculator::EnterEqual()
{
  eestate = false;
	last_input = OPERATION;
	PushStack(&display_data);
	refresh_display = 1;
	
	/*	if (UpdateStack(0))*/
	            UpdateStack(0);
	
	UpdateDisplay();
	precedence_base = 0;
	
	CALCAMNT* number ;

	if(temp_stack.count() > TEMP_STACK_SIZE){ 

	  number = temp_stack.getFirst();
	  temp_stack.removeFirst();

	  if(number)
	    free(number);
	}	

	number = (CALCAMNT*) malloc(sizeof(CALCAMNT));
	*number = DISPLAY_AMOUNT;

	//printf("appending %Lg\n",*number);

	temp_stack.append(number);


}

void QtCalculator::Clear(){

  eestate = false;

  input_count = 0;
  decimal_point = 0;

  if(last_input == OPERATION){
    //    printf("LAST_INPUT = OPERATION\n");
    last_input = DIGIT;
    PopStack();
  }
  else{
    //    printf("LAST_INPUT = DIGIT\n");
    last_input = DIGIT;
  }
             

  if( display_error){
    display_error = 0;
    refresh_display = 0;
  }

  if (!refresh_display) {
    DISPLAY_AMOUNT = 0L;
    UpdateDisplay();
  }

}

void QtCalculator::ClearAll()
{

  eestate = false;
  // last_input = OPERATION;
  last_input = DIGIT;
  RefreshCalculator();
  refresh_display = 1;

}



void QtCalculator::UpdateDisplay()
{

  // this needs to be rewritten based on whether we are currently 
  // inputting a number so that the period and the 0 after a period
  // are correctly displayed.

	CALCAMNT	boh_work_d;
	long 		boh_work = 0;
	int		str_size = 0;

	if(eestate && (current_base == NB_DECIMAL)){
	  
		calc_display->setText(display_str);
		return;
	}
	  
	if (current_base != NB_DECIMAL) { 
		MODF(DISPLAY_AMOUNT, &boh_work_d);
		if (boh_work_d < LONG_MIN || boh_work_d > ULONG_MAX)
		display_error = 1;

	/*
         * We may be in that never-never land where boh numbers
	 * turn from positive to negative - if so then we do
         * just that, allowing boh negative numbers to be entered
         * as read (from dumps and the like!)
         */    
		else if (boh_work_d > LONG_MAX) {
			DISPLAY_AMOUNT = 
				LONG_MIN+(boh_work_d-LONG_MAX-1);
			boh_work = (long)DISPLAY_AMOUNT;
                }
		else {
			DISPLAY_AMOUNT = boh_work_d;
			boh_work = (long) boh_work_d;
		}
	}

	if (!display_error) {

		if (current_base == NB_BINARY)
			str_size = cvb(display_str, 
			  	       boh_work, 
				       BOH_SIZE);
		else if (current_base == NB_OCTAL)
			str_size = sprintf(display_str, 
					   "%lo", 
					   boh_work);
		else if (current_base == NB_DECIMAL) {

		  if(!kcalcdefaults.fixed || last_input == DIGIT
		     || (DISPLAY_AMOUNT > 1.0e+16)){

		    // if I don't guard against the DISPLAY_AMOUNT being too large
		    // kcalc will segfault on larger amount. Such as from typing
		    // from 5*5*******
		    
		    str_size = sprintf(display_str, 

#ifdef HAVE_LONG_DOUBLE
				     "%.*Lg", // was *Lg
				  
				     kcalcdefaults.precision  +1, 
#else
					   "%.*g",

				     kcalcdefaults.precision  +1,
#endif
					     DISPLAY_AMOUNT);
		  }
		  else{//fixed

		  str_size = sprintf(display_str, 

#ifdef HAVE_LONG_DOUBLE
				     "%.*Lf", // was *Lg
				  
				     kcalcdefaults.fixedprecision  , 
#else
					   "%.*f",

				     kcalcdefaults.fixedprecision  ,
#endif
					     DISPLAY_AMOUNT);

		  }// fixed

		  if ( input_count > 0 && !strpbrk(display_str,"e") &&
					   last_input == DIGIT   ) {

#ifdef HAVE_LONG_DOUBLE
		    str_size = sprintf(display_str, 
					   "%.*Lf",
			    (kcalcdefaults.precision +1 > input_count)? 
			     input_count : kcalcdefaults.precision ,
			     DISPLAY_AMOUNT);
#else
		    str_size = sprintf(display_str, 
					   "%.*f",
			    (kcalcdefaults.precision +1 > input_count)? 
			     input_count : kcalcdefaults.precision ,
			     DISPLAY_AMOUNT);
#endif
		  }

		}
		else 
		  if (current_base == NB_HEX)
			str_size = sprintf(display_str, 
					   "%lX", 
					   boh_work);
		  else
			display_error = 1;
	      }

	if (display_error || str_size < 0) { 
	  display_error = 1;
	  strcpy(display_str,"Error");
	  if(kcalcdefaults.beep)
	    QApplication::beep();
	}
	calc_display->setText(display_str);


  if (inverse){
    statusINVLabel->setText("INV");
  }
  else{
    statusINVLabel->setText("NORM");
  }

  if (hyp_mode){
    statusHYPLabel->setText("HYP");
  }
  else{
    statusHYPLabel->setText("");
  }


}

int cvb(char *out_str, long amount, int max_digits)
{
	/*
	 * A routine that converts a long int to 
	 * binary display format
	 */

	char		work_str[(sizeof(amount) * CHAR_BIT) + 1];
	int		work_char = 0, 
			lead_zero = 1, 
			lead_one = 1,
			lead_one_count = 0,
			work_size = sizeof(amount) * CHAR_BIT;
	unsigned long	bit_mask = (1 << ((sizeof(amount) * CHAR_BIT) - 1));

	while (bit_mask) {

		if (amount & bit_mask) {
			if (lead_one)
				lead_one_count++;
			lead_zero = 0;
			work_str[work_char++] = '1';
		} else {
			lead_one = 0;
			if (!lead_zero) 
				work_str[work_char++] = '0';
		}
		bit_mask >>= 1;
	}
	if (!work_char)
		work_str[work_char++] = '0';
	work_str[work_char] = '\0';

	if (work_char-lead_one_count < max_digits)
		return strlen(strcpy(out_str, 
				      &work_str[lead_one_count ? 
				      work_size - max_digits :
				      0]));
	else
	   return -1; 
}		

int UpdateStack(int run_precedence)
{
	item_contents 	new_item, *top_item , *top_function;
	CALCAMNT	left_op =0.0 , right_op =0.0;
	int		op_function= 0, return_value = 0;

	new_item.s_item_type = ITEM_AMOUNT;
	while ((top_function = TopTypeStack(ITEM_FUNCTION)) &&
	top_function->s_item_data.item_func_data.item_precedence >= 
	run_precedence) {

		return_value = 1;

		if ((top_item = PopStack())->s_item_type != ITEM_AMOUNT){
		  QMessageBox::message( "Error", 
					"Stack processing error - right_op", "O.K." );

		}
		right_op = top_item->s_item_data.item_amount;

		if (!((top_item = PopStack()) && 
		top_item->s_item_type == ITEM_FUNCTION)) {
		  QMessageBox::message( "Error", 
					"Stack processing error - function", "O.K." );

		}
		op_function = 
			top_item->s_item_data.item_func_data.item_function;	

		if (!((top_item = PopStack()) && 
		top_item->s_item_type == ITEM_AMOUNT)) {
		  QMessageBox::message( "Error", 
					"Stack processing error - left_op", "O.K." );
		}
		left_op = top_item->s_item_data.item_amount;
	
		new_item.s_item_data.item_amount = 
			(Arith_ops[op_function])(left_op, right_op);
		PushStack(&new_item); 
			
	}	
	if (return_value && 
	    percent_mode && 
	    !display_error &&
	    Prcnt_ops[op_function] != NULL){
		new_item.s_item_data.item_amount =
			(Prcnt_ops[op_function])(left_op,
				right_op, 
				new_item.s_item_data.item_amount); 
		PushStack(&new_item);
	} 
	if (return_value)
		DISPLAY_AMOUNT = new_item.s_item_data.item_amount;

	return return_value;
}

int isoddint(CALCAMNT input) 
{
	CALCAMNT	dummy;
	/*
	 * Routine to check if CALCAMNT is an Odd integer
	 */
	return (MODF(input, &dummy) == 0.0 &&
		MODF(input/2, &dummy) == 0.5);
}

CALCAMNT ExecOr(CALCAMNT left_op, CALCAMNT right_op)
{
  // printf("ExecOr\n");
	CALCAMNT	boh_work_d;
	long 		boh_work_l, boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX) {
		display_error = 1;    
		return 0;
	}
	boh_work_l = (long int)boh_work_d;
	MODF(right_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX) {
		display_error = 1;    
		return 0;
	}
	boh_work_r = (long int) boh_work_d;
	return (boh_work_l | boh_work_r);
}

CALCAMNT ExecXor(CALCAMNT left_op, CALCAMNT right_op)
{
  // printf("ExecXOr\n");
	CALCAMNT	boh_work_d;
	long 		boh_work_l, boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX) {
		display_error = 1;    
		return 0;
	}
	boh_work_l = (long int) boh_work_d;
	MODF(right_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX) {
		display_error = 1;    
		return 0;
	}
	boh_work_r = (long int) boh_work_d;
	return (boh_work_l ^ boh_work_r);
}

CALCAMNT ExecAnd(CALCAMNT left_op, CALCAMNT right_op)
{
  // printf("ExecAnd\n");
	CALCAMNT	boh_work_d;
	long 		boh_work_l, boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX) {
		display_error = 1;    
		return 0;
	}
	boh_work_l = (long int ) boh_work_d;
	MODF(right_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX) {
		display_error = 1;    
		return 0;
	}
	boh_work_r = (long int ) boh_work_d;
	return (boh_work_l & boh_work_r);
}

CALCAMNT ExecLsh(CALCAMNT left_op, CALCAMNT right_op)
{
  // printf("ExecLsh\n");
	CALCAMNT	boh_work_d;
	long 		boh_work_l, boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX) {
		display_error = 1;    
		return 0;
	}
	boh_work_l = (long int) boh_work_d;
	MODF(right_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX) {
		display_error = 1;    
		return 0;
	}
	boh_work_r = (long int ) boh_work_d;
	return (boh_work_l << boh_work_r);
}

CALCAMNT ExecRsh(CALCAMNT left_op, CALCAMNT right_op)
{
  // printf("ExecRsh\n");
	CALCAMNT	boh_work_d;
	long 		boh_work_l, boh_work_r;

	MODF(left_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX) {
		display_error = 1;    
		return 0;
	}
	boh_work_l = (long int ) boh_work_d;
	MODF(right_op, &boh_work_d);
	if (FABS(boh_work_d) > LONG_MAX) {
		display_error = 1;    
		return 0;
	}
	boh_work_r = ( long int ) boh_work_d;
	return (boh_work_l >> boh_work_r);
}

CALCAMNT ExecAdd(CALCAMNT left_op, CALCAMNT right_op)
{
  // printf("ExecAdd\n");
	return left_op + right_op;
}

CALCAMNT ExecSubtract(CALCAMNT left_op, CALCAMNT right_op)
{
  // printf("ExecSubtract\n");
	return left_op - right_op;
}

CALCAMNT ExecMultiply(CALCAMNT left_op, CALCAMNT right_op)
{
  // printf("ExecMulti\n");
	return left_op * right_op;
}

CALCAMNT ExecDivide(CALCAMNT left_op, CALCAMNT right_op)
{
  // printf("ExecDivide\n");
	if (right_op == 0) {
		display_error = 1;
		return 0L;
	} else 
		return left_op / right_op;
}

CALCAMNT ExecMod(CALCAMNT left_op, CALCAMNT right_op)
{
  // printf("ExecMod\n");
  CALCAMNT temp =0.0;

  if (right_op == 0) {
    display_error = 1;
    return 0L;
  } else {

    // x mod y should be the same as x mod -y, thus:
    right_op = FABS(right_op);

    temp = FMOD(left_op, right_op);

    // let's make sure that -7 mod 3 = 2 and NOT -1. 
    // In other words we wand x mod 3 to be a _positive_ number
    // that is 0,1 or 2.

    if( temp < 0 ) temp = right_op + temp;
    temp = FABS(temp);

    return temp;
  }
}

CALCAMNT ExecIntDiv(CALCAMNT left_op, CALCAMNT right_op)
{
  // printf("IndDiv\n");
	if (right_op == 0) {
		display_error = 1;
		return 0L;
	} else {
		MODF(left_op / right_op, &left_op); 
		return left_op;
	}
}

CALCAMNT ExecPower(CALCAMNT left_op, CALCAMNT right_op)
{

       // printf("ExecPowser %g left_op, %g right_op\n",left_op, right_op);
	if (right_op == 0)
		return 1L;
	if (left_op < 0 && isoddint(1/right_op)) 
		left_op = -1L * POW((-1L * left_op), right_op);
	else
		left_op = POW(left_op, right_op);
	if (errno == EDOM || errno == ERANGE) {
		display_error = 1;
		return 0;
	} else 
		return left_op;
}

CALCAMNT ExecPwrRoot(CALCAMNT left_op, CALCAMNT right_op)
{

       // printf("ExecPwrRoot  %g left_op, %g right_op\n", left_op, right_op);
	if (right_op == 0) {
		display_error = 1;
		return 0L;
	}
	if (left_op < 0 && isoddint(right_op)) 
		left_op = -1L * POW((-1L * left_op), (1L)/right_op);
	else
		left_op = POW(left_op, (1L)/right_op);
	if (errno == EDOM || errno == ERANGE) {
	  display_error = 1;
	  return 0;
	}   
	else 
	  return left_op;
}

	
CALCAMNT ExecAddSubP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result)
{
  // printf("ExecAddsubP\n");
  (void) left_op;

	if (result == 0) {
		display_error = 1;
		return 0;
	} else 
		return (result * 100L) / right_op;
}

CALCAMNT ExecMultiplyP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result)
{
  // printf("ExecMultiplyP\n");
  (void) left_op;
  (void) right_op;

	return (result / 100L);
}

CALCAMNT ExecDivideP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result)
{
  // printf("ExecDivideP\n");
  (void) left_op;
  (void) right_op;

	return (result * 100L);
}

CALCAMNT ExecPowerP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result)
{
  // printf("ExecPowerP\n");
  (void) result;
	return ExecPower(left_op, (right_op / 100L));
}

CALCAMNT ExecPwrRootP(CALCAMNT left_op, CALCAMNT right_op, CALCAMNT result)
{
  // printf("ExePwrRootP\n");
  (void) result;

	if (right_op == 0) {
		display_error = 1;
		return 0;
	} else 
		return ExecPower(left_op, (100L / right_op));
}



stack_ptr AllocStackItem (void) {

	if (stack_next <= stack_last) {

		process_stack[stack_next].prior_item = NULL;
		process_stack[stack_next].prior_type = NULL;
		return &process_stack[stack_next++];
	}

	QMessageBox::message( "Emergency", "Stack Error !", "O.K." );
	return &process_stack[stack_next];
}

void UnAllocStackItem (stack_ptr return_item) {

	if (return_item != &process_stack[--stack_next]) {
	
	  QMessageBox::message( "Emergency", "Stack Error !", "O.K." );
	}	

}
void PushStack(item_contents *add_item) 
{
	/*
	 * Add an item to the stack
	 */

	stack_ptr new_item = top_of_stack;

	if (!(new_item && 
	new_item->item_value.s_item_type == add_item->s_item_type)) {

		new_item = AllocStackItem();	/* Get a new item    */

		/*
		 * Chain new item to existing stacks 
		 */

		new_item->prior_item = top_of_stack;
		top_of_stack	     = new_item;
		new_item->prior_type = top_type_stack[add_item->s_item_type];
		top_type_stack[add_item->s_item_type] = new_item;
	}

	new_item->item_value  = *add_item;	/* assign contents*/

}

item_contents *PopStack(void)
{
	/*
	 * Remove and return the top item in the stack
	 */

	static item_contents return_item;
	item_contents *return_item_ptr = NULL;
	stack_ptr return_stack_ptr;

	if ((return_stack_ptr = top_of_stack)) {
		return_item = top_of_stack->item_value;

		top_type_stack[return_item.s_item_type]
			     = top_of_stack->prior_type;
		top_of_stack = top_of_stack->prior_item;

		UnAllocStackItem(return_stack_ptr);
		return_item_ptr = &return_item;
	}

	return return_item_ptr;
}

item_contents *TopOfStack(void)
{
	/*
	 * Return the top item in the stack without removing
	 */

	item_contents *return_item_ptr = NULL;

	if (top_of_stack) {
		return_item_ptr = &top_of_stack->item_value;
	}

	return return_item_ptr;
}

item_contents *TopTypeStack(item_type rqstd_type)
{
	/*
	 * Return the top item in the stack without removing
	 */

	item_contents *return_item_ptr = NULL;

	if (top_type_stack[rqstd_type]) {
		return_item_ptr = &top_type_stack[rqstd_type]->item_value;
	}

	return return_item_ptr;
}


/*
 * Stack storage management Data and Functions
 */



void InitStack (void) {
	
	stack_next = 0;
	stack_last = STACK_SIZE - 1;
	top_of_stack = top_type_stack[0] = top_type_stack[1] = NULL;
}

	
