/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_scaled_brush.h"

KisScaledBrush::KisScaledBrush()
{
    m_mask = 0;
    m_image = QImage();
    m_scale = 1;
    m_xScale = 1;
    m_yScale = 1;
}

KisScaledBrush::KisScaledBrush(KisQImagemaskSP scaledMask, const QImage& scaledImage, double scale, double xScale, double yScale)
{
    m_mask = scaledMask;
    m_image = scaledImage;
    m_scale = scale;
    m_xScale = xScale;
    m_yScale = yScale;

    if (!m_image.isNull()) {
        // Convert image to pre-multiplied by alpha.

        m_image.detach();
        for (int y = 0; y < m_image.height(); y++) {

            QRgb *imagePixelPtr = reinterpret_cast<QRgb *>(m_image.scanLine(y));
            const QRgb *scanline = reinterpret_cast<const QRgb *>(scaledImage.scanLine(y));

            for (int x = 0; x < m_image.width(); x++) {
                QRgb pixel = scanline[x];
                int red = qRed(pixel);
                int green = qGreen(pixel);
                int blue = qBlue(pixel);
                int alpha = qAlpha(pixel);

                red = (red * alpha) / 255;
                green = (green * alpha) / 255;
                blue = (blue * alpha) / 255;

                *imagePixelPtr = qRgba(red, green, blue, alpha);
                ++imagePixelPtr;
            }
        }

    }
}
