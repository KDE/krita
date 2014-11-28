/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_psd_layer_style.h"

#include <QIODevice>

#include <psd.h>
#include <psd_utils.h>

struct KisPSDLayerStyle::Private
{
    Private()
        : version(-1)
        , effects_count(0)
        , visible(false)
    {}
    quint16 version;
    quint8 effects_count; // Effects count: may be 6 (for the 6 effects in Photoshop 5 and 6) or 7 (for Photoshop 7.0)
    bool visible; // common state info, visible: always true
    psd_layer_effects_drop_shadow drop_shadow;
    psd_layer_effects_inner_shadow inner_shadow;
    psd_layer_effects_outer_glow outer_glow;
    psd_layer_effects_inner_glow inner_glow;
    psd_layer_effects_bevel_emboss bevel_emboss;
    psd_layer_effects_satin satin;
    psd_layer_effects_color_overlay color_overlay;
    psd_layer_effects_gradient_overlay gradient_overlay;
    psd_layer_effects_pattern_overlay pattern_overlay;
    psd_layer_effects_stroke stroke;

    QVector<bool> fill;
    QVector<bool> valid;
    QVector<QString> blend_mode;
    QVector<quint8> opacity;
    QVector<QColor> *image_data;
    QVector<qint32> left;
    QVector<qint32> top;
    QVector<qint32> right;
    QVector<qint32> bottom;
    QVector<qint32> width;
    QVector<qint32> height;

};

KisPSDLayerStyle::KisPSDLayerStyle(QObject *parent)
    : QObject(parent)
    , d(new Private())
{
}

bool KisPSDLayerStyle::writeASL(QIODevice *io) const
{
    return false;
}

bool KisPSDLayerStyle::readASL(QIODevice *io)
{
    quint32 tag;

    return false;
}

bool KisPSDLayerStyle::write(QIODevice *io) const
{
    return false;
}

bool KisPSDLayerStyle::read(QIODevice *io)
{
    return false;
}
