/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2020-2021 L. E. Segovia <amy@amyspark.me>
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

enum class OutputMode { InPlace, NewLayers, NewActiveLayers, NewImage, Unspecified = 100 };

// this enum is also index in LAYER_MODE_STRINGS list
enum class InputLayerMode {
    NoInput,
    Active,
    All,
    ActiveAndBelow,
    ActiveAndAbove,
    AllVisible,
    AllInvisible,
    AllVisiblesDesc_DEPRECATED, /* Removed since 2.8.2 */
    AllInvisiblesDesc_DEPRECATED, /* Removed since 2.8.2 */
    AllDesc_DEPRECATED, /* Removed since 2.8.2 */
    Unspecified = 100
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
