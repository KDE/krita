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
#ifndef BRUSH_H
#define BRUSH_H

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <math.h>
//#include "Python.h"

#include <QtCore>

#include "brushsettings.hpp"
#include "helpers.hpp"
#include "mapping.hpp"

#define ACTUAL_RADIUS_MIN 0.2
#define ACTUAL_RADIUS_MAX 800 // safety guard against radius like 1e20 and against rendering overload with unexpected brush dynamics

/* The Brush class stores two things:
   b) settings: constant during a stroke (eg. size, spacing, dynamics, color selected by the user)
   a) states: modified during a stroke (eg. speed, smudge colors, time/distance to next dab, position filter states)

   FIXME: Actually those are two orthogonal things. Should separate them:
          a) brush settings class that is saved/loaded/selected  (without states)
          b) brush core class to draw the dabs (using an instance of the above)

   In python, there are two kinds of instances from this: a "global
   brush" which does the cursor tracking, and the "brushlist" where
   the states are ignored. When a brush is selected, its settings are
   copied into the global one, leaving the state intact.
 */


class Brush {
public:
    bool print_inputs; // debug menu
    // for stroke splitting (undo/redo)
    double stroke_total_painting_time;
    double stroke_current_idling_time;

private:
    // see also brushsettings.py

    // the states (get_state, set_state, reset) that change during a stroke
    float states[STATE_COUNT];
    GRand * rng;

    // Those mappings describe how to calculate the current value for each setting.
    // Most of settings will be constant (eg. only their base_value is used).
    Mapping * settings[BRUSH_SETTINGS_COUNT];

    // the current value of all settings (calculated using the current state)
    float settings_value[BRUSH_SETTINGS_COUNT];

    // cached calculation results
    float speed_mapping_gamma[2], speed_mapping_m[2], speed_mapping_q[2];

public:
    Brush() {

        for (int i=0; i<BRUSH_SETTINGS_COUNT; i++) {
            settings[i] = new Mapping(INPUT_COUNT);
        }
        rng = g_rand_new();
        print_inputs = false;

        for (int i=0; i<STATE_COUNT; i++) {
            states[i] = 0;
        }
        new_stroke();

        settings_base_values_have_changed();
    }

    ~Brush() {
        for (int i=0; i<BRUSH_SETTINGS_COUNT; i++) {
            delete settings[i];
        }
        g_rand_free (rng); rng = NULL;
    }

    void new_stroke()
    {
        stroke_current_idling_time = 0;
        stroke_total_painting_time = 0;
    }

    void set_base_value (int id, float value) {
        g_assert (id >= 0 && id < BRUSH_SETTINGS_COUNT);
        settings[id]->base_value = value;

        settings_base_values_have_changed ();
    }

    void set_mapping_n (int id, int input, int n) {
        g_assert (id >= 0 && id < BRUSH_SETTINGS_COUNT);
        settings[id]->set_n (input, n);
    }

    void set_mapping_point (int id, int input, int index, float x, float y) {
        g_assert (id >= 0 && id < BRUSH_SETTINGS_COUNT);
        settings[id]->set_point (input, index, x, y);
    }

private:
    // returns the fraction still left after t seconds
    float exp_decay (float T_const, float t)
    {
        // the argument might not make mathematical sense (whatever.)
        if (T_const <= 0.001) {
            return 0.0;
        } else {
            return exp(- t / T_const);
        }
    }


    void settings_base_values_have_changed ()
    {
        // precalculate stuff that does not change dynamically

        // Precalculate how the physical speed will be mapped to the speed input value.
        // The forumla for this mapping is:
        //
        // y = log(gamma+x)*m + q;
        //
        // x: the physical speed (pixels per basic dab radius)
        // y: the speed input that will be reported
        // gamma: parameter set by ths user (small means a logarithmic mapping, big linear)
        // m, q: parameters to scale and translate the curve
        //
        // The code below calculates m and q given gamma and two hardcoded constraints.
        //
        for (int i=0; i<2; i++) {
            float gamma;
            gamma = settings[(i==0)?BRUSH_SPEED1_GAMMA:BRUSH_SPEED2_GAMMA]->base_value;
            gamma = exp(gamma);

            float fix1_x, fix1_y, fix2_x, fix2_dy;
            fix1_x = 45.0;
            fix1_y = 0.5;
            fix2_x = 45.0;
            fix2_dy = 0.015;

            float m, q;
            float c1;
            c1 = log(fix1_x+gamma);
            m = fix2_dy * (fix2_x + gamma);
            q = fix1_y - m*c1;

            speed_mapping_gamma[i] = gamma;
            speed_mapping_m[i] = m;
            speed_mapping_q[i] = q;
        }
    }

    // This function runs a brush "simulation" step. Usually it is
    // called once or twice per dab. In theory the precision of the
    // "simulation" gets better when it is called more often. In
    // practice this only matters if there are some highly nonlinear
    // mappings in critical places or extremely few events per second.
    //
    // note: parameters are is dx/ddab, ..., dtime/ddab (dab is the number, 5.0 = 5th dab)
    void update_states_and_setting_values (float step_dx, float step_dy, float step_dpressure, float step_dtime)
    {
        float pressure;
        float inputs[INPUT_COUNT];

        if (step_dtime < 0.0) {
            printf("Time is running backwards!\n");
            step_dtime = 0.001;
        } else if (step_dtime == 0.0) {
            // FIXME: happens about every 10th start, workaround (against division by zero)
            step_dtime = 0.001;
        }

        states[STATE_X]        += step_dx;
        states[STATE_Y]        += step_dy;
        states[STATE_PRESSURE] += step_dpressure;

        float base_radius = expf(settings[BRUSH_RADIUS_LOGARITHMIC]->base_value);

        // FIXME: does happen (interpolation problem?)
        states[STATE_PRESSURE] = CLAMP(states[STATE_PRESSURE], 0.0, 1.0);
        pressure = states[STATE_PRESSURE];

        { // start / end stroke (for "stroke" input only)
            if (!states[STATE_STROKE_STARTED]) {
                if (pressure > settings[BRUSH_STROKE_TRESHOLD]->base_value + 0.0001) {
                    // start new stroke
                    //printf("stroke start %f\n", pressure);
                    states[STATE_STROKE_STARTED] = 1;
                    states[STATE_STROKE] = 0.0;
                }
            } else {
                if (pressure <= settings[BRUSH_STROKE_TRESHOLD]->base_value * 0.9 + 0.0001) {
                    // end stroke
                    //printf("stroke end\n");
                    states[STATE_STROKE_STARTED] = 0;
                }
            }
        }

        // now follows input handling

        float norm_dx, norm_dy, norm_dist, norm_speed;
        norm_dx = step_dx / step_dtime / base_radius;
        norm_dy = step_dy / step_dtime / base_radius;
        norm_speed = sqrt(SQR(norm_dx) + SQR(norm_dy));
        norm_dist = norm_speed * step_dtime;

        inputs[INPUT_PRESSURE] = pressure;
        inputs[INPUT_SPEED1] = log(speed_mapping_gamma[0] + states[STATE_NORM_SPEED1_SLOW])*speed_mapping_m[0] + speed_mapping_q[0];
        inputs[INPUT_SPEED2] = log(speed_mapping_gamma[1] + states[STATE_NORM_SPEED2_SLOW])*speed_mapping_m[1] + speed_mapping_q[1];
        inputs[INPUT_RANDOM] = g_rand_double (rng);
        inputs[INPUT_STROKE] = MIN(states[STATE_STROKE], 1.0);
        //if (states[STATE_NORM_DY_SLOW] == 0 && states[STATE_NORM_DX_SLOW] == 0) {
        //inputs[INPUT_ANGLE] = fmodf(atan2f (states[STATE_NORM_DY_SLOW], states[STATE_NORM_DX_SLOW])/(M_PI) + 1.0, 1.0);
        inputs[INPUT_CUSTOM] = states[STATE_CUSTOM_INPUT];
        if (print_inputs) {
            g_print("press=% 4.3f, speed1=% 4.4f\tspeed2=% 4.4f\tstroke=% 4.3f\tcustom=% 4.3f\n", (double)inputs[INPUT_PRESSURE], (double)inputs[INPUT_SPEED1], (double)inputs[INPUT_SPEED2], (double)inputs[INPUT_STROKE], (double)inputs[INPUT_CUSTOM]);
        }
        // FIXME: this one fails!!!
        //assert(inputs[INPUT_SPEED1] >= 0.0 && inputs[INPUT_SPEED1] < 1e8); // checking for inf

        for (int i=0; i<BRUSH_SETTINGS_COUNT; i++) {
            settings_value[i] = settings[i]->calculate (inputs);
        }

        {
            float fac = 1.0 - exp_decay (settings_value[BRUSH_SLOW_TRACKING_PER_DAB], 1.0);
            states[STATE_ACTUAL_X] += (states[STATE_X] - states[STATE_ACTUAL_X]) * fac; // FIXME: should this depend on base radius?
            states[STATE_ACTUAL_Y] += (states[STATE_Y] - states[STATE_ACTUAL_Y]) * fac;
        }

        { // slow speed
            float fac;
            fac = 1.0 - exp_decay (settings_value[BRUSH_SPEED1_SLOWNESS], step_dtime);
            states[STATE_NORM_SPEED1_SLOW] += (norm_speed - states[STATE_NORM_SPEED1_SLOW]) * fac;
            fac = 1.0 - exp_decay (settings_value[BRUSH_SPEED2_SLOWNESS], step_dtime);
            states[STATE_NORM_SPEED2_SLOW] += (norm_speed - states[STATE_NORM_SPEED2_SLOW]) * fac;
        }

        { // slow speed, but as vector this time
            float fac = 1.0 - exp_decay (exp(settings_value[BRUSH_OFFSET_BY_SPEED_SLOWNESS]*0.01)-1.0, step_dtime);
            states[STATE_NORM_DX_SLOW] += (norm_dx - states[STATE_NORM_DX_SLOW]) * fac;
            states[STATE_NORM_DY_SLOW] += (norm_dy - states[STATE_NORM_DY_SLOW]) * fac;
        }

        { // custom input
            float fac;
            fac = 1.0 - exp_decay (settings_value[BRUSH_CUSTOM_INPUT_SLOWNESS], 0.1);
            states[STATE_CUSTOM_INPUT] += (settings_value[BRUSH_CUSTOM_INPUT] - states[STATE_CUSTOM_INPUT]) * fac;
        }

        { // stroke length
            float frequency;
            float wrap;
            frequency = expf(-settings_value[BRUSH_STROKE_DURATION_LOGARITHMIC]);
            states[STATE_STROKE] += norm_dist * frequency;
            // can happen, probably caused by rounding
            if (states[STATE_STROKE] < 0) states[STATE_STROKE] = 0;
            wrap = 1.0 + settings_value[BRUSH_STROKE_HOLDTIME];
            if (states[STATE_STROKE] > wrap) {
                if (wrap > 9.9 + 1.0) {
                    // "inifinity", just hold stroke somewhere >= 1.0
                    states[STATE_STROKE] = 1.0;
                } else {
                    states[STATE_STROKE] = fmodf(states[STATE_STROKE], wrap);
                    // just in case
                    if (states[STATE_STROKE] < 0) states[STATE_STROKE] = 0;
                }
            }
        }

        // calculate final radius
        float radius_log;
        radius_log = settings_value[BRUSH_RADIUS_LOGARITHMIC];
        states[STATE_ACTUAL_RADIUS] = expf(radius_log);
        if (states[STATE_ACTUAL_RADIUS] < ACTUAL_RADIUS_MIN) states[STATE_ACTUAL_RADIUS] = ACTUAL_RADIUS_MIN;
        if (states[STATE_ACTUAL_RADIUS] > ACTUAL_RADIUS_MAX) states[STATE_ACTUAL_RADIUS] = ACTUAL_RADIUS_MAX;

        // aspect ratio (needs to be caluclated here because it can affect the dab spacing)

        float ratio = settings_value[BRUSH_ELLIPTICAL_DAB_RATIO];
        //float angle = atan2(states[STATE_NORM_DY_SLOW], -states[STATE_NORM_DX_SLOW]) / M_PI * 180;
        float angle = settings_value[BRUSH_ELLIPTICAL_DAB_ANGLE];
        states[STATE_ACTUAL_ELLIPTICAL_DAB_RATIO] = ratio;
        states[STATE_ACTUAL_ELLIPTICAL_DAB_ANGLE] = angle;
    }

    // Called only from stroke_to(). Calculate everything needed to
    // draw the dab, then let the surface do the actual drawing.
    //
    // This is only gets called right after update_states_and_setting_values().
    // Returns true if the surface was modified.
    bool prepare_and_draw_dab (Surface * surface)
    {
        float x, y, opaque;
        float radius;

        opaque = settings_value[BRUSH_OPAQUE] * settings_value[BRUSH_OPAQUE_MULTIPLY];
        if (opaque >= 1.0) opaque = 1.0;
        if (opaque <= 0.0) opaque = 0.0;
        //if (opaque == 0.0) return false; <-- cannot do that, since we need to update smudge state.
        if (settings_value[BRUSH_OPAQUE_LINEARIZE]) {
            // OPTIMIZE: no need to recalculate this for each dab
            float alpha, beta, alpha_dab, beta_dab;
            float dabs_per_pixel;
            // dabs_per_pixel is just estimated roughly, I didn't think hard
            // about the case when the radius changes during the stroke
            dabs_per_pixel = (
                    settings[BRUSH_DABS_PER_ACTUAL_RADIUS]->base_value +
                    settings[BRUSH_DABS_PER_BASIC_RADIUS]->base_value
                    ) * 2.0;

            // the correction is probably not wanted if the dabs don't overlap
            if (dabs_per_pixel < 1.0) dabs_per_pixel = 1.0;

            // interpret the user-setting smoothly
            dabs_per_pixel = 1.0 + settings[BRUSH_OPAQUE_LINEARIZE]->base_value*(dabs_per_pixel-1.0);

            // see doc/brushdab_saturation.png
            //      beta = beta_dab^dabs_per_pixel
            // <==> beta_dab = beta^(1/dabs_per_pixel)
            alpha = opaque;
            beta = 1.0-alpha;
            beta_dab = powf(beta, 1.0/dabs_per_pixel);
            alpha_dab = 1.0-beta_dab;
            opaque = alpha_dab;
        }

        x = states[STATE_ACTUAL_X];
        y = states[STATE_ACTUAL_Y];

        float base_radius = expf(settings[BRUSH_RADIUS_LOGARITHMIC]->base_value);

        if (settings_value[BRUSH_OFFSET_BY_SPEED]) {
            x += states[STATE_NORM_DX_SLOW] * settings_value[BRUSH_OFFSET_BY_SPEED] * 0.1 * base_radius;
            y += states[STATE_NORM_DY_SLOW] * settings_value[BRUSH_OFFSET_BY_SPEED] * 0.1 * base_radius;
        }

        if (settings_value[BRUSH_OFFSET_BY_RANDOM]) {
            x += rand_gauss (rng) * settings_value[BRUSH_OFFSET_BY_RANDOM] * base_radius;
            y += rand_gauss (rng) * settings_value[BRUSH_OFFSET_BY_RANDOM] * base_radius;
        }


        radius = states[STATE_ACTUAL_RADIUS];
        if (settings_value[BRUSH_RADIUS_BY_RANDOM]) {
            float radius_log, alpha_correction;
            // go back to logarithmic radius to add the noise
            radius_log  = settings_value[BRUSH_RADIUS_LOGARITHMIC];
            radius_log += rand_gauss (rng) * settings_value[BRUSH_RADIUS_BY_RANDOM];
            radius = expf(radius_log);
            if (radius < ACTUAL_RADIUS_MIN) radius = ACTUAL_RADIUS_MIN;
            if (radius > ACTUAL_RADIUS_MAX) radius = ACTUAL_RADIUS_MAX;
            alpha_correction = states[STATE_ACTUAL_RADIUS] / radius;
            alpha_correction = SQR(alpha_correction);
            if (alpha_correction <= 1.0) {
                opaque *= alpha_correction;
            }
        }

        // color part

        float color_h = settings[BRUSH_COLOR_H]->base_value;
        float color_s = settings[BRUSH_COLOR_S]->base_value;
        float color_v = settings[BRUSH_COLOR_V]->base_value;
        float eraser_target_alpha = 1.0;
        if (settings_value[BRUSH_SMUDGE] > 0.0) {
            // mix (in RGB) the smudge color with the brush color
            hsv_to_rgb_float (&color_h, &color_s, &color_v);
            float fac = settings_value[BRUSH_SMUDGE];
            if (fac > 1.0) fac = 1.0;
            // If the smudge color somewhat transparent, then the resulting
            // dab will do erasing towards that transparency level.
            // see also ../doc/smudge_math.png
            eraser_target_alpha = (1-fac)*1.0 + fac*states[STATE_SMUDGE_A];
            // fix rounding errors (they really seem to happen in the previous line)
            eraser_target_alpha = CLAMP(eraser_target_alpha, 0.0, 1.0);
            if (eraser_target_alpha > 0) {
                color_h = (fac*states[STATE_SMUDGE_RA] + (1-fac)*color_h) / eraser_target_alpha;
                color_s = (fac*states[STATE_SMUDGE_GA] + (1-fac)*color_s) / eraser_target_alpha;
                color_v = (fac*states[STATE_SMUDGE_BA] + (1-fac)*color_v) / eraser_target_alpha;
            } else {
                // we are only erasing; the color does not matter
                color_h = 1.0;
                color_s = 0.0;
                color_v = 0.0;
            }
            rgb_to_hsv_float (&color_h, &color_s, &color_v);
        }

        if (settings_value[BRUSH_SMUDGE_LENGTH] < 1.0 &&
            // optimization, since normal brushes have smudge_length == 0.5 without actually smudging
            (settings_value[BRUSH_SMUDGE] != 0.0 || !settings[BRUSH_SMUDGE]->is_constant())) {
            float fac = settings_value[BRUSH_SMUDGE_LENGTH];
            if (fac < 0.0) fac = 0;
            int px, py;
            px = ROUND(x);
            py = ROUND(y);
            float r, g, b, a;
            surface->get_color (px, py, radius, &r, &g, &b, &a);
            // updated the smudge color (stored with premultiplied alpha)
            states[STATE_SMUDGE_A ] = fac*states[STATE_SMUDGE_A ] + (1-fac)*a;
            // fix rounding errors
            states[STATE_SMUDGE_A ] = CLAMP(states[STATE_SMUDGE_A], 0.0, 1.0);

            states[STATE_SMUDGE_RA] = fac*states[STATE_SMUDGE_RA] + (1-fac)*r*a;
            states[STATE_SMUDGE_GA] = fac*states[STATE_SMUDGE_GA] + (1-fac)*g*a;
            states[STATE_SMUDGE_BA] = fac*states[STATE_SMUDGE_BA] + (1-fac)*b*a;
        }

        // eraser
        if (settings_value[BRUSH_ERASER]) {
            eraser_target_alpha *= (1.0-settings_value[BRUSH_ERASER]);
        }

        // HSV color change
        color_h += settings_value[BRUSH_CHANGE_COLOR_H];
        color_s += settings_value[BRUSH_CHANGE_COLOR_HSV_S];
        color_v += settings_value[BRUSH_CHANGE_COLOR_V];

        // HSL color change
        if (settings_value[BRUSH_CHANGE_COLOR_L] || settings_value[BRUSH_CHANGE_COLOR_HSL_S]) {
            // (calculating way too much here, can be optimized if neccessary)
            // this function will CLAMP the inputs
            hsv_to_rgb_float (&color_h, &color_s, &color_v);
            rgb_to_hsl_float (&color_h, &color_s, &color_v);
            color_v += settings_value[BRUSH_CHANGE_COLOR_L];
            color_s += settings_value[BRUSH_CHANGE_COLOR_HSL_S];
            hsl_to_rgb_float (&color_h, &color_s, &color_v);
            rgb_to_hsv_float (&color_h, &color_s, &color_v);
        }

        float hardness = settings_value[BRUSH_HARDNESS];

        // the functions below will CLAMP most inputs
        hsv_to_rgb_float (&color_h, &color_s, &color_v);
        return surface->draw_dab (x, y, radius, color_h, color_s, color_v, opaque, hardness, eraser_target_alpha,
                                  states[STATE_ACTUAL_ELLIPTICAL_DAB_RATIO], states[STATE_ACTUAL_ELLIPTICAL_DAB_ANGLE]);
    }

    // How many dabs will be drawn between the current and the next (x, y, pressure, +dt) position?
    float count_dabs_to (float x, float y, float pressure, float dt)
    {
        Q_UNUSED(pressure);
        float xx, yy;
        float res1, res2, res3;
        float dist;

        if (states[STATE_ACTUAL_RADIUS] == 0.0) states[STATE_ACTUAL_RADIUS] = expf(settings[BRUSH_RADIUS_LOGARITHMIC]->base_value);
        if (states[STATE_ACTUAL_RADIUS] < ACTUAL_RADIUS_MIN) states[STATE_ACTUAL_RADIUS] = ACTUAL_RADIUS_MIN;
        if (states[STATE_ACTUAL_RADIUS] > ACTUAL_RADIUS_MAX) states[STATE_ACTUAL_RADIUS] = ACTUAL_RADIUS_MAX;


        // OPTIMIZE: expf() called too often
        float base_radius = expf(settings[BRUSH_RADIUS_LOGARITHMIC]->base_value);
        if (base_radius < ACTUAL_RADIUS_MIN) base_radius = ACTUAL_RADIUS_MIN;
        if (base_radius > ACTUAL_RADIUS_MAX) base_radius = ACTUAL_RADIUS_MAX;
        //if (base_radius < 0.5) base_radius = 0.5;
        //if (base_radius > 500.0) base_radius = 500.0;

        xx = x - states[STATE_X];
        yy = y - states[STATE_Y];
        //dp = pressure - pressure; // Not useful?
        // TODO: control rate with pressure (dabs per pressure) (dpressure is useless)

        if (states[STATE_ACTUAL_ELLIPTICAL_DAB_RATIO] > 1.0) {
            // code duplication, see tiledsurface::draw_dab()
            float angle_rad=states[STATE_ACTUAL_ELLIPTICAL_DAB_ANGLE]*2*M_PI;
            float cs=cos(angle_rad);
            float sn=sin(angle_rad);
            float yyr=(yy*cs-xx*sn)*states[STATE_ACTUAL_ELLIPTICAL_DAB_RATIO];
            float xxr=yy*sn+xx*cs;
            dist = sqrt(yyr*yyr + xxr*xxr);
        } else {
            dist = hypotf(xx, yy);
        }

        // FIXME: no need for base_value or for the range checks above IF always the interpolation
        //        function will be called before this one
        res1 = dist / states[STATE_ACTUAL_RADIUS] * settings[BRUSH_DABS_PER_ACTUAL_RADIUS]->base_value;
        res2 = dist / base_radius   * settings[BRUSH_DABS_PER_BASIC_RADIUS]->base_value;
        res3 = dt * settings[BRUSH_DABS_PER_SECOND]->base_value;
        return res1 + res2 + res3;
    }

public:
    // This function:
    // - is called once for each motion event
    // - does motion event interpolation
    // - paints zero, one or several dabs
    // - decides whether the stroke is finished (for undo/redo)
    // returns true if the stroke is finished or empty
    bool stroke_to (Surface * surface, float x, float y, float pressure, double dtime)
    {
       // printf("%f %f %f %f\n", (double)dtime, (double)x, (double)y, (double)pressure);

        assert(pressure >= 0.0 && pressure <= 1.0);
        assert(x < 1e8 && y < 1e8 && x > -1e8 && y > -1e8);

        if (dtime < 0) g_print("Time jumped backwards by dtime=%f seconds!\n", dtime);
        if (dtime <= 0) dtime = 0.0001; // protect against possible division by zero bugs

        if (dtime > 0.100 && pressure && states[STATE_PRESSURE] == 0) {
            // Workaround for tablets that don't report motion events without pressure.
            // This is to avoid linear interpolation of the pressure between two events.
            stroke_to (surface, x, y, 0.0, dtime-0.0001);
            dtime = 0.0001;
        }

        g_rand_set_seed (rng, states[STATE_RNG_SEED]);

        { // calculate the actual "virtual" cursor position

            // noise first
            if (settings[BRUSH_TRACKING_NOISE]->base_value) {
                // OPTIMIZE: expf() called too often
                float base_radius = expf(settings[BRUSH_RADIUS_LOGARITHMIC]->base_value);

                x += rand_gauss (rng) * settings[BRUSH_TRACKING_NOISE]->base_value * base_radius;
                y += rand_gauss (rng) * settings[BRUSH_TRACKING_NOISE]->base_value * base_radius;
            }

            float fac = 1.0 - exp_decay (settings[BRUSH_SLOW_TRACKING]->base_value, 100.0*dtime);
            x = states[STATE_X] + (x - states[STATE_X]) * fac;
            y = states[STATE_Y] + (y - states[STATE_Y]) * fac;
        }

        // draw many (or zero) dabs to the next position

        // see doc/stroke2dabs.png
        float dist_moved = states[STATE_DIST];
        float dist_todo = count_dabs_to (x, y, pressure, dtime);

        //if (dtime > 5 || dist_todo > 300) {
        if (dtime > 5) {

            /*
        TODO:
        if (dist_todo > 300) {
        // this happens quite often, eg when moving the cursor back into the window
        // FIXME: bad to hardcode a distance treshold here - might look at zoomed image
        //        better detect leaving/entering the window and reset then.
        g_print ("Warning: NOT drawing %f dabs.\n", dist_todo);
        g_print ("dtime=%f, dx=%f\n", dtime, x-states[STATE_X]);
        //must_reset = 1;
        }
      */

            //printf("Brush reset.\n");
            for (int i=0; i<STATE_COUNT; i++) {
                states[i] = 0;
            }

            states[STATE_X] = x;
            states[STATE_Y] = y;
            states[STATE_PRESSURE] = pressure;

            // not resetting, because they will get overwritten below:
            //dx, dy, dpress, dtime

            states[STATE_ACTUAL_X] = states[STATE_X];
            states[STATE_ACTUAL_Y] = states[STATE_Y];
            states[STATE_STROKE] = 1.0; // start in a state as if the stroke was long finished

            return true;
        }

        //g_print("dist = %f\n", states[STATE_DIST]);
        enum { UNKNOWN, YES, NO } painted = UNKNOWN;
        double dtime_left = dtime;

        float step_dx, step_dy, step_dpressure, step_dtime;
        while (dist_moved + dist_todo >= 1.0) { // there are dabs pending
            { // linear interpolation (nonlinear variant was too slow, see SVN log)
                float frac; // fraction of the remaining distance to move
                if (dist_moved > 0) {
                    // "move" the brush exactly to the first dab (moving less than one dab)
                    frac = (1.0 - dist_moved) / dist_todo;
                    dist_moved = 0;
                } else {
                    // "move" the brush from one dab to the next
                    frac = 1.0 / dist_todo;
                }
                step_dx        = frac * (x - states[STATE_X]);
                step_dy        = frac * (y - states[STATE_Y]);
                step_dpressure = frac * (pressure - states[STATE_PRESSURE]);
                step_dtime     = frac * (dtime_left - 0.0);
                // Though it looks different, time is interpolated exactly like x/y/pressure.
            }

            update_states_and_setting_values (step_dx, step_dy, step_dpressure, step_dtime);
            bool painted_now = prepare_and_draw_dab (surface);
            if (painted_now) {
                painted = YES;
            } else if (painted == UNKNOWN) {
                painted = NO;
            }

            dtime_left   -= step_dtime;
            dist_todo  = count_dabs_to (x, y, pressure, dtime_left);
        }

        {
            // "move" the brush to the current time (no more dab will happen)
            // Important to do this at least once every event, because
            // brush_count_dabs_to depends on the radius and the radius can
            // depend on something that changes much faster than only every
            // dab (eg speed).

            step_dx        = x - states[STATE_X];
            step_dy        = y - states[STATE_Y];
            step_dpressure = pressure - states[STATE_PRESSURE];
            step_dtime     = dtime_left;

            //dtime_left = 0; but that value is not used any more

            update_states_and_setting_values (step_dx, step_dy, step_dpressure, step_dtime);
        }

        // save the fraction of a dab that is already done now
        states[STATE_DIST] = dist_moved + dist_todo;
        //g_print("dist_final = %f\n", states[STATE_DIST]);

        // next seed for the RNG (GRand has no get_state() and states[] must always contain our full state)
        states[STATE_RNG_SEED] = g_rand_int(rng);

        // stroke separation logic (for undo/redo)

        if (painted == UNKNOWN) {
            if (stroke_current_idling_time > 0 || stroke_total_painting_time == 0) {
                // still idling
                painted = NO;
            } else {
                // probably still painting (we get more events than brushdabs)
                painted = YES;
                //if (pressure == 0) g_print ("info: assuming 'still painting' while there is no pressure\n");
            }
        }
        if (painted == YES) {
            //if (stroke_current_idling_time > 0) g_print ("idling ==> painting\n");
            stroke_total_painting_time += dtime;
            stroke_current_idling_time = 0;
            // force a stroke split after some time
            if (stroke_total_painting_time > 4 + 3*pressure) {
                // but only if pressure is not being released
                // FIXME: use some smoothed state for dpressure, not the output of the interpolation code
                //        (which might easily wrongly give dpressure == 0)
                if (step_dpressure >= 0) {
                    return true;
                }
            }
        } else if (painted == NO) {
            //if (stroke_current_idling_time == 0) g_print ("painting ==> idling\n");
            stroke_current_idling_time += dtime;
            if (stroke_total_painting_time == 0) {
                // not yet painted, start a new stroke if we have accumulated a lot of irrelevant motion events
                if (stroke_current_idling_time > 1.0) {
                    return true;
                }
            } else {
                // Usually we have pressure==0 here. But some brushes can paint
                // nothing at full pressure (eg gappy lines, or a stroke that
                // fades out). In either case this is the prefered moment to split.
                if (stroke_total_painting_time+stroke_current_idling_time > 1.2 + 5*pressure) {
                    return true;
                }
            }
        }
        return false;
    }

#if 0
    PyObject * get_state ()
    {
        npy_intp dims = {STATE_COUNT};
        PyObject * data = PyArray_SimpleNew(1, &dims, NPY_FLOAT32);
        npy_float32 * data_p = (npy_float32*)PyArray_DATA(data);
        for (int i=0; i<STATE_COUNT; i++) {
            data_p[i] = states[i];
        }
        return data;
    }

    void set_state (PyObject * data)
    {
        assert(PyArray_NDIM(data) == 1);
        assert(PyArray_DIM(data, 0) == STATE_COUNT);
        assert(PyArray_ISCARRAY(data));
        npy_float32 * data_p = (npy_float32*)PyArray_DATA(data);
        for (int i=0; i<STATE_COUNT; i++) {
            states[i] = data_p[i];
        }
    }
#endif

    double get_actual_radius() {

        double base_radius = expf(settings[BRUSH_RADIUS_LOGARITHMIC]->base_value);
        //double base_radius = states[STATE_ACTUAL_RADIUS];

        double r = base_radius;

        r += 2*base_radius*settings[BRUSH_OFFSET_BY_RANDOM]->base_value;
        //r += 2*base_radius*settings_value[BRUSH_OFFSET_BY_RANDOM];

        return r;
    }
};

#endif
