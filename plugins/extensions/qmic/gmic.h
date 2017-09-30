/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef GMIC_H
#define GMIC_H

/**
 * A sham header to make it easier to handle gmic types without
 * needing gmic itself.
 */

enum OutputMode {   IN_PLACE = 0,
                        NEW_LAYERS,
                        NEW_ACTIVE_LAYERS,
                        NEW_IMAGE
};


// this enum is also index in LAYER_MODE_STRINGS list
enum InputLayerMode {   NONE = 0,
                        ACTIVE_LAYER,
                        ALL_LAYERS,
                        ACTIVE_LAYER_BELOW_LAYER,
                        ACTIVE_LAYER_ABOVE_LAYER,
                        ALL_VISIBLE_LAYERS,
                        ALL_INVISIBLE_LAYERS,
                        ALL_VISIBLE_LAYERS_DECR,
                        ALL_INVISIBLE_DECR,
                        ALL_DECR
};


template<typename T> struct gmic_image {
    unsigned int _width;       // Number of image columns (dimension along the X-axis).
    unsigned int _height;      // Number of image lines (dimension along the Y-axis)
    unsigned int _depth;       // Number of image slices (dimension along the Z-axis).
    unsigned int _spectrum;    // Number of image channels (dimension along the C-axis).
    bool _is_shared;           // Tells if the data buffer is shared by another structure.
    T *_data;                  // Pointer to the first pixel value.
    QString name;              // Layer name

    void assign(unsigned int w, unsigned int h, unsigned int d, unsigned int s) {
        _width = w;
        _height = h;
        _depth = d;
        _spectrum = s;

    }
};

#endif // GMIC_H
