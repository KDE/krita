/*
 * SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * Code inspired by quantizeRGBprimsS15Fixed16 in ArgyllCMS 'icc.c' file:
 * SPDX-FileCopyrightText: 1997-2013 Graeme W. Gill
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef COLORPROFILEQUANTIZATION_H
#define COLORPROFILEQUANTIZATION_H

#include <cmath>
#include <QVector>
#include <KoAlwaysInline.h>

#endif // COLORPROFILEQUANTIZATION_H


/*
 * Implementation of quantization, referenced from
 * quantizeRGBprimsS15Fixed16 in ArgyllCMS 'icc.c' file.
 * Quantization is necessary to reduce errors in the resulting icc profile,
 * as icc profiles always store with 16bit precision. Using this function,
 * we can reduce that error to 6 digits behind the comma.
 */
ALWAYS_INLINE void quantizexyYPrimariesTo16bit(QVector<double> &colorants) {
    for (int i=0; i<colorants.size(); i += 3) {
        QVector<double> value (3);

        // We convert not to XYZ, but to xyz, that is, z is normalized.
        // That way this code doesn't default to Y being the largest value.
        value[0] = colorants[i];
        value[1] = colorants[i+1];
        value[2] = 1.0 - value[0] - value[1];

        // Because things are normalized, the sum is always 1.0.
        double sum = 1.0;
        double largest = -1e9;
        int largestValue = 0;

        for (int j = 0; j < value.size(); j++) {
            double val = value.at(j);
            if (val>largest) {
                largest = val;
                largestValue = j;
            }
            value[j] = floor(val * 65536.0 + 0.5) / 65536.0;
        }

        for (int j = 0; j < value.size(); j++) {
            if (j == largestValue) {
                continue;
            }
            sum -= value.at(j);
        }

        value[largestValue] = floor(sum * 65536.0 + 0.5) / 65536.0;

        colorants[i] = value[0];
        colorants[i+1] = value[1];
    }
}
