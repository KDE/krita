/*
 * Copyright 2009 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _WACOM_PROPERTIES_H_
#define _WACOM_PROPERTIES_H_

/**
 * Properties exported by the wacom driver. These properties are
 * recognized by the driver and will change its behavior when modified.
 */

/* 32 bit, 4 values, top x, top y, bottom x, bottom y */
#define WACOM_PROP_TABLET_AREA "Wacom Tablet Area"

/* 8 bit, 1 value, [0 - 3] (NONE, CW, CCW, HALF) */
#define WACOM_PROP_ROTATION "Wacom Rotation"

/* 32 bit, 4 values */
#define WACOM_PROP_PRESSURECURVE "Wacom Pressurecurve"

/* CARD32, 5 values, tablet id, old serial, old hw device id,
   current serial, current device id
   read-only
 */
#define WACOM_PROP_SERIALIDS "Wacom Serial IDs"

/* CARD32, 1 value */
#define WACOM_PROP_SERIAL_BIND "Wacom Serial ID binding"

/* 8 bit, 4 values, left up, left down, right up, right down
   OR
   Atom, 4 values , left up, left down, right up, right down
  */
#define WACOM_PROP_STRIPBUTTONS "Wacom Strip Buttons"

/* 8 bit, 6 values, rel wheel up, rel wheel down, abs wheel up, abs wheel down, abs wheel 2 up, abs wheel 2 down
   OR
   Atom, 6 values , rel wheel up, rel wheel down, abs wheel up, abs wheel down, abs wheel 2 up, abs wheel 2 down
 */
#define WACOM_PROP_WHEELBUTTONS "Wacom Wheel Buttons"

/* DEPRECATED, DO NOT USE */
#define WACOM_PROP_TWINVIEW_RES "Wacom TwinView Resolution"

/* DEPRECATED. DO NOT USE */
#define WACOM_PROP_DISPLAY_OPTS "Wacom Display Options"

/* DEPRECATED. DO NOT USE */
#define WACOM_PROP_SCREENAREA "Wacom Screen Area"

/* 32 bit, 1 value */
#define WACOM_PROP_PROXIMITY_THRESHOLD "Wacom Proximity Threshold"

/* DEPRECATED. DO NOT USE */
#define WACOM_PROP_CAPACITY "Wacom Capacity"

/* 32 bit, 1 value */
#define WACOM_PROP_PRESSURE_THRESHOLD "Wacom Pressure Threshold"

/* 32 bit, 2 values, suppress, sample */
#define WACOM_PROP_SAMPLE "Wacom Sample and Suppress"

/* BOOL, 1 value */
#define WACOM_PROP_TOUCH "Wacom Enable Touch"

/* BOOL, 1 value, read-only */
#define WACOM_PROP_HARDWARE_TOUCH "Wacom Hardware Touch Switch"

/* 8 bit, 1 values */
#define WACOM_PROP_ENABLE_GESTURE "Wacom Enable Touch Gesture"

/* 32 bit, 3 values, zoom, rotate, tap parameters */
#define WACOM_PROP_GESTURE_PARAMETERS "Wacom Touch Gesture Parameters"

/* BOOL, 1 value,
   TRUE == hover click is enabled, FALSE == hover click disabled */
#define WACOM_PROP_HOVER "Wacom Hover Click"

/* Atom, 1 value, read-only */
#define WACOM_PROP_TOOL_TYPE "Wacom Tool Type"

/* Atom, X values where X is the number of physical buttons.
   Each value points to an atom containing the sequence of actions performed
   if this button is pressed. If the value is None, no action is performed.
 */
#define WACOM_PROP_BUTTON_ACTIONS "Wacom Button Actions"

/* 8 bit, 2 values, priv->debugLevel and common->debugLevel. This property
 * is for use in the driver only and only enabled if --enable-debug is
 * given. No client may rely on this property being present or working.
 */
#define WACOM_PROP_DEBUGLEVELS "Wacom Debug Levels"

/* BOOL, 1 value,
   TRUE == pressure renormalization enabled, FALSE == pressure renormalization disabled
*/
#define WACOM_PROP_PRESSURE_RECAL "Wacom Pressure Recalibration"

/* The following are tool types used by the driver in WACOM_PROP_TOOL_TYPE
 * or in the 'type' field for XI1 clients. Clients may check for one of
 * these types to identify tool types.
 */
#define WACOM_PROP_XI_TYPE_STYLUS "STYLUS"
#define WACOM_PROP_XI_TYPE_CURSOR "CURSOR"
#define WACOM_PROP_XI_TYPE_ERASER "ERASER"
#define WACOM_PROP_XI_TYPE_PAD    "PAD"
#define WACOM_PROP_XI_TYPE_TOUCH  "TOUCH"

#endif
