/*
    $Id$

    KCalc, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no

    Copyright (C) 1996 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

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


#include "stats.h"
#include <stdio.h>
#include <stdlib.h>

KStats::KStats(){

  error_flag = FALSE;
  data.setAutoDelete(TRUE);

}

KStats::~KStats(){

  data.clear();

}

void KStats::clearAll(){

  data.clear();

}

void KStats::enterData(CALCAMNT _data){

  CALCAMNT *newdata;
  newdata = new CALCAMNT;
  *newdata = _data;
  data.append(newdata);

#ifdef DEBUG_STATS
  printf("Added %Lg\n",*newdata);
  printf("count %d\n",data.count());
#endif

}


void KStats::clearLast(){


 data.removeLast();
#ifdef DEBUG_STATS
printf("count %d\n",data.count());
#endif


}

CALCAMNT KStats::sum(){

  CALCAMNT result = 0.0;
  CALCAMNT *dp;
  for ( dp=data.first(); dp != 0; dp=data.next() ){

    result += *dp;

  }

#ifdef DEBUG_STATS
  printf("Sum %Lg\n",result);
#endif

  return result;
}

CALCAMNT KStats::min()
{
  printf("MIIINNNN\n");

  if ( data.count() == 0 )
    return 0.0;

  printf("1\n");

  CALCAMNT result = *(data.first());
  printf("result=%f\n",result);

  CALCAMNT *dp = data.next();
  for ( ; dp != 0; dp=data.next() )
    if ( *dp < result )
      result = *dp;

  printf("Return\n");

  return result;
}

CALCAMNT KStats::max()
{
  if ( data.count() == 0 )
    return 0.0;

  CALCAMNT result = *(data.first());
  CALCAMNT *dp = data.next();
  for ( ; dp != 0; dp=data.next() )
    if ( *dp > result )
      result = *dp;

  return result;
}

CALCAMNT KStats::mul(){

  CALCAMNT result = 1.0;
  CALCAMNT *dp;
  for ( dp=data.first(); dp != 0; dp=data.next() ){

    result *= *dp;

  }

  return result;
}

CALCAMNT KStats::median(){

  int index;
  CALCAMNT result;
  CALCAMNT *dp;
  int bound = 0;

  MyList list;

  for ( dp=data.first(); dp != 0; dp=data.next() ){
    list.inSort(dp);
  }

#ifdef DEBUG_STATS
  for(int l = 0; l < (int)list.count();l++){

    printf("Sorted %Lg\n",*list.at(l));

  }
#endif

  bound = list.count();

  if (bound == 0){
    error_flag = TRUE;
    return 0.0;
  }

  if ( bound == 1)
    return *list.at(0);

  if( bound % 2){  // odd

    index = (bound - 1 ) / 2 + 1;
    result =  *list.at(index - 1 );
  }
  else { // even

    index = bound / 2;
    result = ((*list.at(index - 1))  + (*list.at(index)))/2;
 }

  return result;

}



CALCAMNT KStats::std_kernel(){

  CALCAMNT result = 0.0;
  CALCAMNT _mean;

  _mean = mean();

  CALCAMNT *dp;
  for ( dp=data.first(); dp != 0; dp=data.next() ){

    result += (*dp - _mean) * (*dp - _mean);

  }

#ifdef DEBUG_STATS
  printf("std_kernel %Lg\n",result);
#endif

  return result;

}


CALCAMNT KStats::sum_of_squares(){

  CALCAMNT result = 0.0;
  CALCAMNT *dp;
  for ( dp=data.first(); dp != 0; dp=data.next() ){

    result += (*dp) * (*dp);

  }

#ifdef DEBUG_STATS
  printf("Sum of Squares %Lg\n",result);
#endif

  return result;

}

CALCAMNT KStats::mean(){

  CALCAMNT result = 0.0;

  if(data.count() == 0){
    error_flag = TRUE;
    return 0.0;
  }

  result = sum()/data.count();

#ifdef DEBUG_STATS
  printf("mean: %Lg\n",result);
#endif

  return result;

}

CALCAMNT KStats::std(){

  CALCAMNT result = 0.0;

  if(data.count() == 0){
    error_flag = TRUE;

#ifdef DEBUG_STATS
    printf("set stats error\n");
#endif

    return 0.0;
  }

  result = SQRT(std_kernel());

#ifdef DEBUG_STATS
  printf ("data.count %d\n",data.count());
#endif

  result = result/data.count();

#ifdef DEBUG_STATS
  printf("std: %Lg\n",result);
#endif

  return result;

}

CALCAMNT KStats::sample_std(){

  CALCAMNT result = 0.0;

  if(data.count() < 2 ){
    error_flag = TRUE;
    return 0.0;
  }

  result = SQRT(std_kernel());
  result = result/(data.count() - 1);
#ifdef DEBUG_STATS
  printf("sample std: %Lg\n",result);
#endif
  return result;

}

int  KStats::count(){

  return data.count();

}

bool KStats::error(){

  bool value;
  value = error_flag;
  error_flag = FALSE;

  return value;

}

int MyList::compareItems(Item item_1, Item item_2){

  CALCAMNT *item1;
  CALCAMNT *item2;

  item1 = (CALCAMNT*) item_1;
  item2 = (CALCAMNT*) item_2;

  if(*item1 > *item2)
    return 1;

  if(*item2 > *item1)
    return -1;

  return 0;

}



