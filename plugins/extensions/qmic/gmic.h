/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef GMIC_H
#define GMIC_H

#include <QString>

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
enum InputLayerMode {
    NONE = 0,
    ACTIVE_LAYER,
    ALL_LAYERS,
    ACTIVE_LAYER_BELOW_LAYER,
    ACTIVE_LAYER_ABOVE_LAYER,
    ALL_VISIBLE_LAYERS,
    ALL_INVISIBLE_LAYERS,
    ALL_VISIBLE_LAYERS_DECR_UNUSED, /* Removed since 2.8.2 */
    ALL_INVISIBLE_DECR_UNUSED,      /* Removed since 2.8.2 */
    ALL_DECR_UNUSED,                /* Removed since 2.8.2 */
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
