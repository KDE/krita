/* brushlib - The MyPaint Brush Library
 * Copyright (C) 2007-2008 Martin Renold <martinxyz@gmx.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY. See the COPYING file for more details.
 */

#ifndef __MAPPING_H__
#define __MAPPING_H__

#include "helpers.hpp"

// user-defined mappings
// (the curves you can edit in the brush settings)

class Mapping {
private:
  typedef struct {
    // a set of control points (stepwise linear)
    float xvalues[8];
    float yvalues[8];
    int n;
  } ControlPoints;

  int inputs;
  ControlPoints * pointsList; // one for each input
  int inputs_used; // optimization
public:
  float base_value;

  Mapping(int inputs_) {
    inputs = inputs_;
    pointsList = new ControlPoints[inputs];
    for (int i=0; i<inputs; i++) pointsList[i].n = 0;
    inputs_used = 0;
    base_value = 0;
  }
  ~Mapping() {
    delete pointsList;
  }

  void set_n (int input, int n)
  {
    assert (input >= 0 && input < inputs);
    assert (n >= 0 && n <= 8);
    assert (n != 1); // cannot build a linear mapping with only one point
    ControlPoints * p = pointsList + input;

    if (n != 0 && p->n == 0) inputs_used++;
    if (n == 0 && p->n != 0) inputs_used--;
    assert(inputs_used >= 0);
    assert(inputs_used <= inputs);

    p->n = n;
  }

  void set_point (int input, int index, float x, float y)
  {
    assert (input >= 0 && input < inputs);
    assert (index >= 0 && index < 8);
    ControlPoints * p = pointsList + input;
    assert (index < p->n);

    if (index > 0) {
      assert (x > p->xvalues[index-1]);
    }

    p->xvalues[index] = x;
    p->yvalues[index] = y;
  }

  bool is_constant()
  {
    return inputs_used == 0;
  }

  float calculate (float * data)
  {
    int j;
    float result;
    result = base_value;

    // constant mapping (common case)
    if (inputs_used == 0) return result;

    for (j=0; j<inputs; j++) {
      ControlPoints * p = pointsList + j;

      if (p->n) {
        float x, y;
        x = data[j];

        // find the segment with the slope that we need to use
        float x0, y0, x1, y1;
        x0 = p->xvalues[0];
        y0 = p->yvalues[0];
        x1 = p->xvalues[1];
        y1 = p->yvalues[1];

        int i;
        for (i=2; i<p->n && x>x1; i++) {
          x0 = x1;
          y0 = y1;
          x1 = p->xvalues[i];
          y1 = p->yvalues[i];
        }

        //g_print ("%d/%d x0=%f,x1=%f\n", i, p->n, x0, x1);

        // linear interpolation
        float m, q;
        m = (y1-y0)/(x1-x0);
        q = y0 - m*x0;
        y = m*x + q;
        result += y;
      }
    }
    return result;
  }

  // used in python for the global pressure mapping
  float calculate_single_input (float input)
  {
    assert(inputs == 1); 
    return calculate(&input);
  }
};

#endif
