/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "Selection.h"

#include <KoColorSpace.h>
#include "kis_iterator_ng.h"
#include <kis_selection.h>
#include <kis_pixel_selection.h>
#include <kis_paint_device.h>
#include <kis_selection_filters.h>
#include <kis_painter.h>
#include <kis_clipboard.h>
#include <QByteArray>

#include <Node.h>

struct Selection::Private {
    Private() {}
    KisSelectionSP selection;
};

Selection::Selection(KisSelectionSP selection, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->selection = selection;
}


Selection::Selection(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->selection = new KisSelection();
}

Selection::~Selection()
{
    delete d;
}

bool Selection::operator==(const Selection &other) const
{
    return (d->selection == other.d->selection);
}

bool Selection::operator!=(const Selection &other) const
{
    return !(operator==(other));
}

Selection *Selection::duplicate() const
{
    return new Selection(d->selection ? new KisSelection(*d->selection)
                         : new KisSelection());
}

int Selection::width() const
{
    if (!d->selection) return 0;
    return d->selection->selectedExactRect().width();
}

int Selection::height() const
{
    if (!d->selection) return 0;
    return d->selection->selectedExactRect().height();
}

int Selection::x() const
{
    if (!d->selection) return 0;
    int xPos = d->selection->x();
    if (d->selection->hasNonEmptyPixelSelection()) {
        xPos = d->selection->selectedExactRect().x();
    }
    return xPos;
}

int Selection::y() const
{
    if (!d->selection) return 0;
    int yPos = d->selection->y();
    if (d->selection->hasNonEmptyPixelSelection()) {
        yPos = d->selection->selectedExactRect().y();
    }
    return yPos;
}

void Selection::move(int x, int y)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->moveTo(QPoint(x, y));
}


void Selection::clear()
{
    if (!d->selection) return;
    d->selection->clear();
}

void Selection::contract(int value)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->select(QRect(x(), y(), width() - value, height() - value));
}

void Selection::copy(Node *node)
{
    if (!node) return;
    if (!d->selection) return;
    if (node->node()->exactBounds().isEmpty()) return;
    if (!node->node()->hasEditablePaintDevice()) return;

    KisPaintDeviceSP dev = node->node()->paintDevice();
    KisPaintDeviceSP clip = new KisPaintDevice(dev->colorSpace());
    KisPaintDeviceSP selectionProjection = d->selection->projection();

    const KoColorSpace *cs = clip->colorSpace();
    const KoColorSpace *selCs = d->selection->projection()->colorSpace();

    QRect rc = d->selection->selectedExactRect();

    KisPainter::copyAreaOptimized(QPoint(), dev, clip, rc);

    KisHLineIteratorSP layerIt = clip->createHLineIteratorNG(0, 0, rc.width());
    KisHLineConstIteratorSP selectionIt = selectionProjection->createHLineIteratorNG(rc.x(), rc.y(), rc.width());

    for (qint32 y = 0; y < rc.height(); y++) {
        for (qint32 x = 0; x < rc.width(); x++) {

            qreal dstAlpha = cs->opacityF(layerIt->rawData());
            qreal sel = selCs->opacityF(selectionIt->oldRawData());
            qreal newAlpha = sel * dstAlpha / (1.0 - dstAlpha + sel * dstAlpha);
            float mask = newAlpha / dstAlpha;

            cs->applyAlphaNormedFloatMask(layerIt->rawData(), &mask, 1);

            layerIt->nextPixel();
            selectionIt->nextPixel();
        }
        layerIt->nextRow();
        selectionIt->nextRow();
    }

    KisClipboard::instance()->setClip(clip, rc.topLeft());
}

void Selection::cut(Node* node)
{
    if (!node) return;
    if (!d->selection) return;
    if (node->node()->exactBounds().isEmpty()) return;
    if (!node->node()->hasEditablePaintDevice()) return;
    KisPaintDeviceSP dev = node->node()->paintDevice();
    copy(node);
    dev->clearSelection(d->selection);
    node->node()->setDirty(d->selection->selectedExactRect());
}

void Selection::paste(Node *destination, int x, int y)
{
    if (!destination) return;
    if (!d->selection) return;
    if (!KisClipboard::instance()->hasClip()) return;

    KisPaintDeviceSP src = KisClipboard::instance()->clip(QRect(), false);
    KisPaintDeviceSP dst = destination->node()->paintDevice();
    if (!dst || !src) return;
    KisPainter::copyAreaOptimized(QPoint(x, y),
                                  src,
                                  dst,
                                  src->exactBounds(),
                                  d->selection);
    destination->node()->setDirty();
}

void Selection::erode()
{
    if (!d->selection) return;
    KisErodeSelectionFilter esf;
    QRect rc = esf.changeRect(d->selection->selectedExactRect(), d->selection->pixelSelection()->defaultBounds());
    esf.process(d->selection->pixelSelection(), rc);
}

void Selection::dilate()
{
    if (!d->selection) return;
    KisDilateSelectionFilter dsf;
    QRect rc = dsf.changeRect(d->selection->selectedExactRect(), d->selection->pixelSelection()->defaultBounds());
    dsf.process(d->selection->pixelSelection(), rc);
}

void Selection::border(int xRadius, int yRadius)
{
    if (!d->selection) return;
    KisBorderSelectionFilter sf(xRadius, yRadius, true);
    QRect rc = sf.changeRect(d->selection->selectedExactRect(), d->selection->pixelSelection()->defaultBounds());
    sf.process(d->selection->pixelSelection(), rc);
}

void Selection::feather(int radius)
{
    if (!d->selection) return;
    KisFeatherSelectionFilter fsf(radius);
    QRect rc = fsf.changeRect(d->selection->selectedExactRect(), d->selection->pixelSelection()->defaultBounds());
    fsf.process(d->selection->pixelSelection(), rc);
}

void Selection::grow(int xradius, int yradius)
{
    if (!d->selection) return;
    KisGrowSelectionFilter gsf(xradius, yradius);
    QRect rc = gsf.changeRect(d->selection->selectedExactRect(), d->selection->pixelSelection()->defaultBounds());
    gsf.process(d->selection->pixelSelection(), rc);
}


void Selection::shrink(int xRadius, int yRadius, bool edgeLock)
{
    if (!d->selection) return;
    KisShrinkSelectionFilter sf(xRadius, yRadius, edgeLock);
    QRect rc = sf.changeRect(d->selection->selectedExactRect(), d->selection->pixelSelection()->defaultBounds());
    sf.process(d->selection->pixelSelection(), rc);
}

void Selection::smooth()
{
    if (!d->selection) return;
    KisSmoothSelectionFilter sf;
    QRect rc = sf.changeRect(d->selection->selectedExactRect(), d->selection->pixelSelection()->defaultBounds());
    sf.process(d->selection->pixelSelection(), rc);
}


void Selection::invert()
{
    if (!d->selection) return;
    KisInvertSelectionFilter sf;
    QRect rc = sf.changeRect(d->selection->selectedExactRect(), d->selection->pixelSelection()->defaultBounds());
    sf.process(d->selection->pixelSelection(), rc);
}

void Selection::resize(int w, int h)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->select(QRect(x(), y(), w, h));
}

void Selection::select(int x, int y, int w, int h, int value)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->select(QRect(x, y, w, h), value);
}

void Selection::selectAll(Node *node, int value)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->select(node->node()->exactBounds(), value);
}

void Selection::replace(Selection *selection)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->applySelection(selection->selection()->pixelSelection(), SELECTION_REPLACE);
}

void Selection::add(Selection *selection)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->applySelection(selection->selection()->pixelSelection(), SELECTION_ADD);
}

void Selection::subtract(Selection *selection)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->applySelection(selection->selection()->pixelSelection(), SELECTION_SUBTRACT);
}

void Selection::intersect(Selection *selection)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->applySelection(selection->selection()->pixelSelection(), SELECTION_INTERSECT);
}

void Selection::symmetricdifference(Selection *selection)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->applySelection(selection->selection()->pixelSelection(), SELECTION_SYMMETRICDIFFERENCE);
}


QByteArray Selection::pixelData(int x, int y, int w, int h) const
{
    QByteArray ba;
    if (!d->selection) return ba;
    KisPaintDeviceSP dev = d->selection->projection();
    quint8 *data = new quint8[w * h];
    dev->readBytes(data, x, y, w, h);
    ba = QByteArray((const char*)data, (int)(w * h));
    delete[] data;
    return ba;
}

void Selection::setPixelData(QByteArray value, int x, int y, int w, int h)
{
    if (!d->selection) return;
    KisPixelSelectionSP dev = d->selection->pixelSelection();
    if (!dev) return;
    dev->writeBytes((const quint8*)value.constData(), x, y, w, h);
}

KisSelectionSP Selection::selection() const
{
    return d->selection;
}


