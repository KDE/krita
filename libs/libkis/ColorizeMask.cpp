/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "ColorizeMask.h"
#include <lazybrush/kis_colorize_mask.h>
#include <lazybrush/kis_lazy_fill_tools.h>
#include <kis_image.h>
#include "Selection.h"
#include <kis_selection.h>
#include <KoColor.h>
#include "kis_layer_properties_icons.h"
#include <kis_transaction.h>
#include <kis_update_scheduler.h>
#include <kis_undo_stores.h>
#include <kis_default_bounds.h>



ColorizeMask::ColorizeMask(KisImageSP image, QString name, QObject *parent) :
    Node(image, new KisColorizeMask(image,name), parent)
{

}

ColorizeMask::ColorizeMask(KisImageSP image, KisColorizeMaskSP mask, QObject *parent):
    Node(image, mask, parent)
{
}

ColorizeMask::~ColorizeMask()
{

}


QList<ManagedColor*> ColorizeMask::keyStrokesColors() const
{
    QList<ManagedColor*> colorList;
    const KisColorizeMask *mask = qobject_cast<const KisColorizeMask*>(this->node());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, colorList);

    for (KoColor color : mask->keyStrokesColors().colors) {
        colorList.append(new ManagedColor(color));
    }

    return colorList;
}

void ColorizeMask::initializeKeyStrokeColors(QList<ManagedColor*> colors, int transparentIndex)
{
    KisColorizeMaskSP mask = qobject_cast<KisColorizeMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(mask);

    /**
     *  This method is supposed to to initial initialization only!
     *
     *  It is necessary because the function also changes the color
     *  space and blending mode of the mask
     *
     *  TODO: implement a proper API that modifies key strokes
     *  of a colorize mask without breaking undo history
     */
    KIS_SAFE_ASSERT_RECOVER_RETURN(mask->keyStrokesColors().colors.size() == 0);

    mask->initializeCompositeOp();
    delete mask->setColorSpace(mask->parent()->colorSpace());

    QList<KisLazyFillTools::KeyStroke> keyStrokes;

    for (int i = 0; i < colors.size(); i++) {
        KisLazyFillTools::KeyStroke keyStroke;
        keyStroke.color = colors[i]->color();
        keyStroke.dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
        keyStroke.dev->setDefaultBounds(new KisDefaultBounds(this->node()->image()));
        keyStroke.isTransparent = transparentIndex == i;
        // NOTE: the parent node link is initialized in
        //       setKeyStrokesDirect

        keyStrokes.append(keyStroke);
    }

    mask->setKeyStrokesDirect(keyStrokes);
}


int ColorizeMask::transparencyIndex() const
{
    const KisColorizeMask *mask = qobject_cast<const KisColorizeMask*>(this->node());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, -1);

    return mask->keyStrokesColors().transparentIndex;
}

void ColorizeMask::removeKeyStroke(ManagedColor* color)
{
    const KoColor kc = color->color();
    KisColorizeMask *mask = qobject_cast<KisColorizeMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(mask);

    mask->removeKeyStroke(kc);
}

QByteArray ColorizeMask::keyStrokePixelData(ManagedColor* color, int x, int y, int w, int h) const
{
    QByteArray ba;

    if (!this->node()) return ba;

    const KoColor kc = color->color();
    const KisColorizeMask *mask = qobject_cast<const KisColorizeMask*>(this->node());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, QByteArray());

    for (KisLazyFillTools::KeyStroke keystroke : mask->fetchKeyStrokesDirect()) {
        if (kc == keystroke.color) {
            KisPaintDeviceSP dev = keystroke.dev;

            if (!dev) return ba;

            ba.resize(w * h * dev->pixelSize());
            dev->readBytes(reinterpret_cast<quint8*>(ba.data()), x, y, w, h);
            return ba;
        }
    }

    return ba;
}

bool ColorizeMask::setKeyStrokePixelData(QByteArray value, ManagedColor* color, int x, int y, int w, int h)
{
    if (!this->node()) return false;

    const KoColor kc = color->color();
    const KisColorizeMask *mask = qobject_cast<const KisColorizeMask*>(this->node());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, false);

    for (KisLazyFillTools::KeyStroke keystroke : mask->fetchKeyStrokesDirect()) {
        if (kc == keystroke.color) {
            KisPaintDeviceSP dev = keystroke.dev;

            if (!dev) return false;
            if (value.length() <  w * h * (int)dev->colorSpace()->pixelSize()) {
                qWarning() << "ColorizeMask::setKeyStrokePixelData: not enough data to write to the paint device";
                return false;
            }
            dev->writeBytes((const quint8*)value.constData(), x, y, w, h);
            return true;
        }
    }

    return false;
}

void ColorizeMask::setUseEdgeDetection(bool value)
{
    KisColorizeMask *mask = qobject_cast<KisColorizeMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(mask);

    mask->setUseEdgeDetection(value);
}

bool ColorizeMask::useEdgeDetection() const
{
    const KisColorizeMask *mask = qobject_cast<const KisColorizeMask*>(this->node());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, false);

    return mask->useEdgeDetection();
}

void ColorizeMask::setEdgeDetectionSize(qreal value)
{
    KisColorizeMask *mask = qobject_cast<KisColorizeMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(mask);

    mask->setEdgeDetectionSize(value);
}

qreal ColorizeMask::edgeDetectionSize() const
{
    const KisColorizeMask *mask = qobject_cast<const KisColorizeMask*>(this->node());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, -1);

    return mask->edgeDetectionSize();
}

void ColorizeMask::setCleanUpAmount(qreal value)
{
    KisColorizeMask *mask = qobject_cast<KisColorizeMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(mask);

    mask->setCleanUpAmount(value);
}

qreal ColorizeMask::cleanUpAmount() const
{
    const KisColorizeMask *mask = qobject_cast<const KisColorizeMask*>(this->node());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, -1);

    return mask->cleanUpAmount();
}

void ColorizeMask::setLimitToDeviceBounds(bool value)
{
    KisColorizeMask *mask = qobject_cast<KisColorizeMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(mask);

    mask->setLimitToDeviceBounds(value);
}

bool ColorizeMask::limitToDeviceBounds() const
{
    const KisColorizeMask *mask = qobject_cast<const KisColorizeMask*>(this->node());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, false);

    return mask->limitToDeviceBounds();
}

void ColorizeMask::updateMask(bool force)
{
    KisColorizeMask *mask = qobject_cast<KisColorizeMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(mask);

    if (force) {
        mask->forceRegenerateMask();
    } else {
        KisLayerPropertiesIcons::setNodePropertyAutoUndo(mask, KisLayerPropertiesIcons::colorizeNeedsUpdate, false, this->node()->image());
    }
}

void ColorizeMask::resetCache()
{
    KisColorizeMask *mask = qobject_cast<KisColorizeMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(mask);

    mask->resetCache();
}

void ColorizeMask::setShowOutput(bool enabled)
{
    KisColorizeMask *mask = qobject_cast<KisColorizeMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(mask);

    KisLayerPropertiesIcons::setNodePropertyAutoUndo(mask, KisLayerPropertiesIcons::colorizeShowColoring, enabled, this->node()->image());
}

bool ColorizeMask::showOutput() const
{
    const KisColorizeMask *mask = qobject_cast<const KisColorizeMask*>(this->node());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, false);

    const KisBaseNode::PropertyList props = mask->sectionModelProperties();

    for (const KisBaseNode::Property &prop : props) {
        if (prop.id == KisLayerPropertiesIcons::colorizeShowColoring.id()) {
            return prop.state.toBool();
        }
    }

    return false;
}

void ColorizeMask::setEditKeyStrokes(bool enabled)
{
    KisColorizeMask *mask = qobject_cast<KisColorizeMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(mask);

    KisLayerPropertiesIcons::setNodePropertyAutoUndo(mask, KisLayerPropertiesIcons::colorizeEditKeyStrokes, enabled, this->node()->image());
}

bool ColorizeMask::editKeyStrokes() const
{
    const KisColorizeMask *mask = qobject_cast<const KisColorizeMask*>(this->node());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, false);

    const KisBaseNode::PropertyList props = mask->sectionModelProperties();

    for (const KisBaseNode::Property &prop : props) {
        if (prop.id == KisLayerPropertiesIcons::colorizeEditKeyStrokes.id()) {
            return prop.state.toBool();
        }
    }

    return false;
}

QString ColorizeMask::type() const
{
    return "colorizemask";
}
