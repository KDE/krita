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
#include <QUuid>

#include <psd.h>
#include <psd_utils.h>

#include <klocalizedstring.h>

#include "kis_global.h"


struct Q_DECL_HIDDEN KisPSDLayerStyle::Private
{
    Private()
        : version(-1)
        , effectEnabled(true)
    {}

    Private(const Private &rhs)
        : name(rhs.name),
          uuid(rhs.uuid),
          version(rhs.version),
          effectEnabled(rhs.effectEnabled),
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
            uuid = rhs.uuid;
            version = rhs.version;
            effectEnabled = rhs.effectEnabled;
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
    QUuid uuid;
    quint16 version;
    bool effectEnabled;
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
    : KoResource(QString())
    , d(new Private())
{
    d->name = i18n("Unnamed");
    d->version = 7;
}

KisPSDLayerStyle::~KisPSDLayerStyle()
{
    delete d;
}

KisPSDLayerStyle::KisPSDLayerStyle(const KisPSDLayerStyle &rhs)
    : KoResource(QString())
    , d(new Private(*rhs.d))
{
    setValid(valid());
}

KisPSDLayerStyle KisPSDLayerStyle::operator=(const KisPSDLayerStyle &rhs)
{
    if (this != &rhs) {
        *d = *rhs.d;
    }
    return *this;
}

bool KisPSDLayerStyle::isEnabled() const
{
    return d->effectEnabled;
}

void KisPSDLayerStyle::setEnabled(bool value)
{
    d->effectEnabled = value;
}

KoResourceSP KisPSDLayerStyle::clone() const
{
    return toQShared(new KisPSDLayerStyle(*this)).dynamicCast<KoResource>();
}

void KisPSDLayerStyle::clear()
{
    *d = Private();
}

bool KisPSDLayerStyle::isEmpty() const
{
    return !(d->drop_shadow.effectEnabled() ||
             d->inner_shadow.effectEnabled() ||
             d->outer_glow.effectEnabled() ||
             d->inner_glow.effectEnabled() ||
             d->bevel_emboss.effectEnabled() ||
             d->satin.effectEnabled() ||
             d->color_overlay.effectEnabled() ||
             d->gradient_overlay.effectEnabled() ||
             d->pattern_overlay.effectEnabled() ||
             d->stroke.effectEnabled());
}

QString KisPSDLayerStyle::name() const
{
    return d->name;
}

void KisPSDLayerStyle::setName(const QString &value)
{
    d->name = value;
    dynamic_cast<KoResource*>(this)->setName(value);
}

QUuid KisPSDLayerStyle::uuid() const
{
    if (d->uuid.isNull()) {
        d->uuid = QUuid::createUuid();
    }

    return d->uuid;
}

void KisPSDLayerStyle::setUuid(const QUuid &value) const
{
    d->uuid = value;
}

QString KisPSDLayerStyle::psdUuid() const
{
    return uuid().toString().mid(1, 36);
}

void KisPSDLayerStyle::setPsdUuid(const QString &value) const
{
    setUuid(QUuid(QString("{%1}").arg(value)));
}

bool KisPSDLayerStyle::load()
{
    return true;
}

bool KisPSDLayerStyle::loadFromDevice(QIODevice *dev)
{
    Q_UNUSED(dev);
    return true;
}

bool KisPSDLayerStyle::save()
{
    return true;
}

bool KisPSDLayerStyle::saveToDevice(QIODevice *dev) const
{
    Q_UNUSED(dev);
    return true;
}



const psd_layer_effects_context* KisPSDLayerStyle::context() const
{
    return &d->context;
}

const psd_layer_effects_drop_shadow* KisPSDLayerStyle::dropShadow() const
{
    return &d->drop_shadow;
}

const psd_layer_effects_inner_shadow* KisPSDLayerStyle::innerShadow() const
{
    return &d->inner_shadow;
}

const psd_layer_effects_outer_glow* KisPSDLayerStyle::outerGlow() const
{
    return &d->outer_glow;
}

const psd_layer_effects_inner_glow* KisPSDLayerStyle::innerGlow() const
{
    return &d->inner_glow;
}

const psd_layer_effects_satin* KisPSDLayerStyle::satin() const
{
    return &d->satin;
}

const psd_layer_effects_color_overlay* KisPSDLayerStyle::colorOverlay() const
{
    return &d->color_overlay;
}

const psd_layer_effects_gradient_overlay* KisPSDLayerStyle::gradientOverlay() const
{
    return &d->gradient_overlay;
}

const psd_layer_effects_pattern_overlay* KisPSDLayerStyle::patternOverlay() const
{
    return &d->pattern_overlay;
}

const psd_layer_effects_stroke* KisPSDLayerStyle::stroke() const
{
    return &d->stroke;
}

const psd_layer_effects_bevel_emboss* KisPSDLayerStyle::bevelAndEmboss() const
{
    return &d->bevel_emboss;
}

psd_layer_effects_context* KisPSDLayerStyle::context()
{
    return &d->context;
}

psd_layer_effects_drop_shadow* KisPSDLayerStyle::dropShadow()
{
    return &d->drop_shadow;
}

psd_layer_effects_inner_shadow* KisPSDLayerStyle::innerShadow()
{
    return &d->inner_shadow;
}

psd_layer_effects_outer_glow* KisPSDLayerStyle::outerGlow()
{
    return &d->outer_glow;
}

psd_layer_effects_inner_glow* KisPSDLayerStyle::innerGlow()
{
    return &d->inner_glow;
}

psd_layer_effects_satin* KisPSDLayerStyle::satin()
{
    return &d->satin;
}

psd_layer_effects_color_overlay* KisPSDLayerStyle::colorOverlay()
{
    return &d->color_overlay;
}

psd_layer_effects_gradient_overlay* KisPSDLayerStyle::gradientOverlay()
{
    return &d->gradient_overlay;
}

psd_layer_effects_pattern_overlay* KisPSDLayerStyle::patternOverlay()
{
    return &d->pattern_overlay;
}

psd_layer_effects_stroke* KisPSDLayerStyle::stroke()
{
    return &d->stroke;
}

psd_layer_effects_bevel_emboss* KisPSDLayerStyle::bevelAndEmboss()
{
    return &d->bevel_emboss;
}
