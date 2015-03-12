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

#include <klocale.h>

#include "kis_global.h"


struct KisPSDLayerStyle::Private
{
    Private()
        : version(-1)
        , effects_count(0)
        , visible(false)
    {}

    Private(const Private &rhs)
        : name(rhs.name),
          version(rhs.version),
          effects_count(rhs.effects_count),
          visible(rhs.visible),
          context(rhs.context),
          drop_shadow(rhs.drop_shadow),
          inner_shadow(rhs.inner_shadow),
          outer_glow(rhs.outer_glow),
          inner_glow(rhs.inner_glow),
          bevel_emboss(rhs.bevel_emboss),
          satin(rhs.satin),
          color_overlay(rhs.color_overlay),
          gradient_overlay(rhs.gradient_overlay),
          pattern_overlay(rhs.pattern_overlay),
          stroke(rhs.stroke)
    {}

    Private operator=(const Private &rhs)
    {
        if (this != &rhs) {
            name = rhs.name;
            version = rhs.version;
            effects_count = rhs.effects_count;
            visible = rhs.visible;
            context = rhs.context;
            drop_shadow = rhs.drop_shadow;
            inner_shadow = rhs.inner_shadow;
            outer_glow = rhs.outer_glow;
            inner_glow = rhs.inner_glow;
            bevel_emboss = rhs.bevel_emboss;
            satin = rhs.satin;
            color_overlay = rhs.color_overlay;
            gradient_overlay = rhs.gradient_overlay;
            pattern_overlay = rhs.pattern_overlay;
            stroke = rhs.stroke;
        }

        return *this;
    }

    QString name;
    quint16 version;
    quint8 effects_count; // Effects count: may be 6 (for the 6 effects in Photoshop 5 and 6) or 7 (for Photoshop 7.0)
    bool visible; // common state info, visible: always true
    psd_layer_effects_context context;
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
};

KisPSDLayerStyle::KisPSDLayerStyle()
    : d(new Private())
{
    d->name = i18n("Unnamed");
    d->version = 7;
    d->visible = true;
}

KisPSDLayerStyle::~KisPSDLayerStyle()
{
    delete d;
}

KisPSDLayerStyle::KisPSDLayerStyle(const KisPSDLayerStyle &rhs)
    : d(new Private(*rhs.d))
{
}

KisPSDLayerStyle KisPSDLayerStyle::operator=(const KisPSDLayerStyle &rhs)
{
    if (this != &rhs) {
        *d = *rhs.d;
    }
    return *this;
}

KisPSDLayerStyleSP KisPSDLayerStyle::clone() const
{
    return toQShared(new KisPSDLayerStyle(*this));
}

bool KisPSDLayerStyle::isEmpty() const
{
    return !d->drop_shadow.effect_enable;
}

QString KisPSDLayerStyle::name() const
{
    return d->name;
}

const psd_layer_effects_context* KisPSDLayerStyle::context() const
{
    return &d->context;
}

const psd_layer_effects_drop_shadow* KisPSDLayerStyle::drop_shadow() const
{
    return &d->drop_shadow;
}

psd_layer_effects_context* KisPSDLayerStyle::context()
{
    return &d->context;
}

psd_layer_effects_drop_shadow* KisPSDLayerStyle::drop_shadow()
{
    return &d->drop_shadow;
}

bool KisPSDLayerStyle::writeASL(QIODevice *io, QVector<KisPSDLayerStyle *> )
{
    return false;
}

QVector<KisPSDLayerStyle *> KisPSDLayerStyle::readASL(QIODevice *io)
{
    return QVector<KisPSDLayerStyle*>();
}

bool KisPSDLayerStyle::write(QIODevice *io) const
{
    return false;
}

bool KisPSDLayerStyle::read(QIODevice *io)
{
    return false;
}

psd_layer_effects_drop_shadow &KisPSDLayerStyle::dropShadow() const
{
    return d->drop_shadow;
}

void KisPSDLayerStyle::setDropShadow(const psd_layer_effects_drop_shadow &dropShadow)
{
    d->drop_shadow = dropShadow;
}
