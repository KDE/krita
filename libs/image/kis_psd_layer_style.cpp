/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_psd_layer_style.h"

#include <QBuffer>
#include <QIODevice>
#include <QUuid>
#include <KLocalizedString>

#include <kis_assert.h>
#include <kis_global.h>
#include <psd_utils.h>
#include "krita_container_utils.h"

#include <KoCanvasResourcesInterface.h>
#include <KoAbstractGradient.h>
#include <KisRequiredResourcesOperators.h>
#include <KoMD5Generator.h>

#include "kis_asl_layer_style_serializer.h"


struct Q_DECL_HIDDEN KisPSDLayerStyle::Private
{
    Private(KisResourcesInterfaceSP _resourcesInterface)
        : version(-1)
        , effectEnabled(true)
        , resourcesInterface(_resourcesInterface)
    {
        if (!resourcesInterface) {
            resourcesInterface.reset(new KisLocalStrokeResources({}));
        }
    }

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
          stroke(rhs.stroke),
          resourcesInterface(rhs.resourcesInterface)
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
            resourcesInterface = rhs.resourcesInterface;
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

    KisResourcesInterfaceSP resourcesInterface;
};

KisPSDLayerStyle::KisPSDLayerStyle(const QString &filename, KisResourcesInterfaceSP resourcesInterface)
    : KoResource(filename)
    , d(new Private(resourcesInterface))
{
    d->name = i18n("Unnamed");
    d->version = 7;
}

KisPSDLayerStyle::~KisPSDLayerStyle()
{
    delete d;
}

KisPSDLayerStyle::KisPSDLayerStyle(const KisPSDLayerStyle &rhs)
    : KoResource(rhs)
    , d(new Private(*rhs.d))
{
    setValid(valid());
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

bool KisPSDLayerStyle::isSerializable() const
{
    return false;
}

bool KisPSDLayerStyle::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(dev);
    Q_UNUSED(resourcesInterface);
    return false;
}

bool KisPSDLayerStyle::saveToDevice(QIODevice *) const
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(false && "KisPSDLayerStyle is not meant to be serializable!");
    return false;
}

void KisPSDLayerStyle::clear()
{
    *d = Private(d->resourcesInterface);
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
    const_cast<KisPSDLayerStyle*>(this)->setMD5Sum(KoMD5Generator::generateHash(value.toByteArray()));
}

QString KisPSDLayerStyle::psdUuid() const
{
    return uuid().toString().mid(1, 36);
}

void KisPSDLayerStyle::setPsdUuid(const QString &value) const
{
    setUuid(QUuid(QString("{%1}").arg(value)));
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

KisResourcesInterfaceSP KisPSDLayerStyle::resourcesInterface() const
{
    return d->resourcesInterface;
}

void KisPSDLayerStyle::setResourcesInterface(KisResourcesInterfaceSP resourcesInterface)
{
    d->resourcesInterface = resourcesInterface;
}

bool KisPSDLayerStyle::hasLocalResourcesSnapshot() const
{
    return KisRequiredResourcesOperators::hasLocalResourcesSnapshot(this);
}

KisPSDLayerStyleSP KisPSDLayerStyle::cloneWithResourcesSnapshot(KisResourcesInterfaceSP globalResourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface) const
{
    KisPSDLayerStyleSP style = KisRequiredResourcesOperators::cloneWithResourcesSnapshot<KisPSDLayerStyleSP>(this, globalResourcesInterface);

    /**
     * Passing null into cloneWithResourcesSnapshot() means that we expect all the
     * canvas resources to be backed into the style. That is exactly what we expect
     * when loading the styles saved in the layers.
     */

    if (!requiredCanvasResources().isEmpty() && !canvasResourcesInterface) {
        qWarning() << "KisPSDLayerStyle::cloneWithResourcesSnapshot: layer style"
                   << name() << "is expected to have all the canvas resources"
                   << "backed, but still depends on something";
        qWarning() << ppVar(requiredCanvasResources());
    }

    if (canvasResourcesInterface) {

        QSharedPointer<KisLocalStrokeResources> localResourcesSnapshot =
             style->resourcesInterface().dynamicCast<KisLocalStrokeResources>();
        KIS_ASSERT_RECOVER_RETURN_VALUE(localResourcesSnapshot, style);

        auto bakeGradient = [canvasResourcesInterface, localResourcesSnapshot] (KoAbstractGradientSP gradient) {
            if (gradient && !gradient->requiredCanvasResources().isEmpty()) {

                /**
                 * Since we haven't cloned the required resources when putting them
                 * into the local storage (which is rather questionable), we need to
                 * clone the gradients explicitly before modification.
                 */

                KoAbstractGradientSP clonedGradient =
                    gradient->cloneAndBakeVariableColors(canvasResourcesInterface);

                localResourcesSnapshot->removeResource(gradient);
                localResourcesSnapshot->addResource(clonedGradient);
            }
        };

        if (style->gradientOverlay()->effectEnabled()) {
            bakeGradient(style->gradientOverlay()->gradient(style->resourcesInterface()));
        }

        if (style->innerGlow()->effectEnabled() && style->innerGlow()->fillType() == psd_fill_gradient) {
            bakeGradient(style->innerGlow()->gradient(style->resourcesInterface()));
        }

        if (style->outerGlow()->effectEnabled() && style->outerGlow()->fillType() == psd_fill_gradient) {
            bakeGradient(style->outerGlow()->gradient(style->resourcesInterface()));
        }

        if (style->stroke()->effectEnabled() && style->stroke()->fillType() == psd_fill_gradient) {
            bakeGradient(style->stroke()->gradient(style->resourcesInterface()));
        }
    }

    return style;
}

QList<KoResourceLoadResult> KisPSDLayerStyle::embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    Q_UNUSED(globalResourcesInterface);
    return implicitCastList<KoResourceLoadResult>(QList<KoResourceSP>::fromVector(KisAslLayerStyleSerializer::fetchEmbeddedResources(this)));
}

QList<int> KisPSDLayerStyle::requiredCanvasResources() const
{
    QList<int> result;

    auto addCanvasResources = [&result] (KoAbstractGradientSP gradient) {
        if (gradient) {
            result << gradient->requiredCanvasResources();
        }
    };

    if (gradientOverlay()->effectEnabled()) {
        addCanvasResources(gradientOverlay()->gradient(resourcesInterface()));
    }

    if (innerGlow()->effectEnabled() && innerGlow()->fillType() == psd_fill_gradient) {
        addCanvasResources(innerGlow()->gradient(resourcesInterface()));
    }

    if (outerGlow()->effectEnabled() && outerGlow()->fillType() == psd_fill_gradient) {
        addCanvasResources(outerGlow()->gradient(resourcesInterface()));
    }

    if (stroke()->effectEnabled() && stroke()->fillType() == psd_fill_gradient) {
        addCanvasResources(stroke()->gradient(resourcesInterface()));
    }

    KritaUtils::makeContainerUnique(result);

    return result;
}
