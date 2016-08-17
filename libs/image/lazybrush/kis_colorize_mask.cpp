/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_colorize_mask.h"

#include <QCoreApplication>

#include <KoColorSpaceRegistry.h>
#include "kis_pixel_selection.h"

#include "kis_icon_utils.h"

#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "kis_lazy_fill_tools.h"
#include "kis_cached_paint_device.h"
#include "kis_paint_device_debug_utils.h"
#include "kis_layer_properties_icons.h"
#include "kis_signal_compressor.h"

#include "kis_colorize_job.h"
#include "kis_multiway_cut.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_macro_based_undo_store.h"
#include "kis_post_execution_undo_adapter.h"
#include "kis_command_utils.h"


using namespace KisLazyFillTools;

struct KisColorizeMask::Private
{
    Private()
        : needAddCurrentKeyStroke(false),
          showKeyStrokes(true), showColoring(true), needsUpdate(true),
          updateCompressor(1000, KisSignalCompressor::POSTPONE)
    {
    }

    Private(const Private &rhs)
        : needAddCurrentKeyStroke(rhs.needAddCurrentKeyStroke),
          showKeyStrokes(rhs.showKeyStrokes),
          showColoring(rhs.showColoring),
          cachedSelection(),
          needsUpdate(false),
          updateCompressor(1000, KisSignalCompressor::POSTPONE)
    {
    }

    QList<KeyStroke> keyStrokes;
    KisPaintDeviceSP coloringProjection;
    KisPaintDeviceSP fakePaintDevice;
    KisPaintDeviceSP filteredSource;

    KoColor currentColor;
    KisPaintDeviceSP currentKeyStrokeDevice;
    bool needAddCurrentKeyStroke;

    bool showKeyStrokes;
    bool showColoring;

    KisCachedSelection cachedSelection;
    KisCachedSelection cachedConversionSelection;

    bool needsUpdate;

    KisSignalCompressor updateCompressor;
};

KisColorizeMask::KisColorizeMask()
    : m_d(new Private)
{
    // TODO: correct initialization of the layer's color space
    m_d->fakePaintDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    connect(&m_d->updateCompressor,
            SIGNAL(timeout()),
            SLOT(slotUpdateRegenerateFilling()));

    m_d->updateCompressor.moveToThread(qApp->thread());
}

KisColorizeMask::~KisColorizeMask()
{
}

KisColorizeMask::KisColorizeMask(const KisColorizeMask& rhs)
    : KisEffectMask(rhs),
      m_d(new Private(*rhs.m_d))
{
    connect(&m_d->updateCompressor,
            SIGNAL(timeout()),
            SLOT(slotUpdateRegenerateFilling()));

    m_d->updateCompressor.moveToThread(qApp->thread());
}

bool KisColorizeMask::needsUpdate() const
{
    return m_d->needsUpdate;
}

void KisColorizeMask::setNeedsUpdate(bool value)
{
    if (value != m_d->needsUpdate) {
        m_d->needsUpdate = value;
        baseNodeChangedCallback();

        if (!value) {
            m_d->updateCompressor.start();
        }
    }
}

void KisColorizeMask::slotUpdateRegenerateFilling()
{
    KisPaintDeviceSP src = parent()->original();
    KIS_ASSERT_RECOVER_RETURN(src);

    if (!m_d->coloringProjection) {
        m_d->coloringProjection = new KisPaintDevice(src->colorSpace());
        m_d->filteredSource = new KisPaintDevice(src->colorSpace());
    } else {
        // TODO: sync colorspaces
        m_d->coloringProjection->clear();
        m_d->filteredSource->clear();
    }

    KisLayerSP parentLayer = dynamic_cast<KisLayer*>(parent().data());
    if (!parentLayer) return;

    KisImageSP image = parentLayer->image();
    if (image) {
        KisColorizeJob *job = new KisColorizeJob(src, m_d->coloringProjection, m_d->filteredSource, image->bounds());
        Q_FOREACH (const KeyStroke &stroke, m_d->keyStrokes) {
            job->addKeyStroke(stroke.dev, stroke.color);
        }

        connect(job, SIGNAL(sigFinished()), SLOT(slotRegenerationFinished()));
        image->addSpontaneousJob(job);
    }
}

void KisColorizeMask::slotRegenerationFinished()
{
    setNeedsUpdate(true);
    setDirty();
}

KisBaseNode::PropertyList KisColorizeMask::sectionModelProperties() const
{
    KisBaseNode::PropertyList l = KisMask::sectionModelProperties();
    l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::colorizeNeedsUpdate, needsUpdate());
    l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::colorizeShowKeyStrokes, showKeyStrokes());
    l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::colorizeShowColoring, showColoring());

    return l;
}

void KisColorizeMask::setSectionModelProperties(const KisBaseNode::PropertyList &properties)
{
    KisMask::setSectionModelProperties(properties);

    Q_FOREACH (const KisBaseNode::Property &property, properties) {
        if (property.id == KisLayerPropertiesIcons::colorizeNeedsUpdate.id()) {
            if (m_d->needsUpdate != property.state.toBool()) {
                setNeedsUpdate(property.state.toBool());
            }
        }
        if (property.id == KisLayerPropertiesIcons::colorizeShowKeyStrokes.id()) {
            if (m_d->showKeyStrokes != property.state.toBool()) {
                setShowKeyStrokes(property.state.toBool());
            }
        }
        if (property.id == KisLayerPropertiesIcons::colorizeShowColoring.id()) {
            if (m_d->showColoring != property.state.toBool()) {
                setShowColoring(property.state.toBool());
            }
        }
    }
}

KisPaintDeviceSP KisColorizeMask::paintDevice() const
{
    return m_d->showKeyStrokes ? m_d->fakePaintDevice : 0;
}

QIcon KisColorizeMask::icon() const
{
    return KisIconUtils::loadIcon("filterMask");
}


bool KisColorizeMask::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

void KisColorizeMask::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
}

QRect KisColorizeMask::decorateRect(KisPaintDeviceSP &src,
                                    KisPaintDeviceSP &dst,
                                    const QRect &rect,
                                    PositionToFilthy maskPos) const
{
    Q_UNUSED(maskPos);

    KIS_ASSERT(dst != src);

    // Draw the filling and the original layer
    {
        KisPainter gc(dst);

        if (m_d->showKeyStrokes) {
            gc.setOpacity(128);
        }
        if (m_d->showColoring && m_d->coloringProjection) {
            gc.bitBlt(rect.topLeft(), m_d->coloringProjection, rect);
        }

        if (m_d->showKeyStrokes &&
            m_d->filteredSource &&
            !m_d->filteredSource->extent().isEmpty()) {

            // TODO: the filtered source should be converted back into alpha!
            gc.setOpacity(128);
            gc.bitBlt(rect.topLeft(), m_d->filteredSource, rect);
        } else {
            gc.setOpacity(255);
            gc.bitBlt(rect.topLeft(), src, rect);
        }
    }

    // Draw the key strokes
    if (m_d->showKeyStrokes) {
        lockTemporaryTarget();

        KisSelectionSP selection = m_d->cachedSelection.getSelection();
        KisSelectionSP conversionSelection = m_d->cachedConversionSelection.getSelection();
        KisPixelSelectionSP tempSelection = conversionSelection->pixelSelection();

        KisPaintDeviceSP temporaryTarget = this->temporaryTarget();
        const bool isTemporaryTargetErasing = temporaryCompositeOp() == COMPOSITE_ERASE;
        const QRect temporaryExtent = temporaryTarget ? temporaryTarget->extent() : QRect();


        KisFillPainter gc(dst);

        QList<KeyStroke> extendedStrokes = m_d->keyStrokes;

        if (m_d->currentKeyStrokeDevice &&
            m_d->needAddCurrentKeyStroke &&
            !isTemporaryTargetErasing) {

            extendedStrokes << KeyStroke(m_d->currentKeyStrokeDevice, m_d->currentColor);
        }

        Q_FOREACH (const KeyStroke &stroke, extendedStrokes) {
            selection->pixelSelection()->makeCloneFromRough(stroke.dev, rect);
            gc.setSelection(selection);

            if (stroke.color == m_d->currentColor ||
                (isTemporaryTargetErasing &&
                 temporaryExtent.intersects(selection->pixelSelection()->selectedRect()))) {

                if (temporaryTarget) {
                    tempSelection->copyAlphaFrom(temporaryTarget, rect);

                    KisPainter selectionPainter(selection->pixelSelection());
                    setupTemporaryPainter(&selectionPainter);
                    selectionPainter.bitBlt(rect.topLeft(), tempSelection, rect);
                }
            }

            gc.fillSelection(rect, stroke.color);
        }

        m_d->cachedSelection.putSelection(selection);
        m_d->cachedSelection.putSelection(conversionSelection);

        unlockTemporaryTarget();
    }

    return rect;
}

QRect KisColorizeMask::extent() const
{
    QRect rc;

    // TODO: take care about the filtered device, which can be painted
    //       semi-transparent sometimes

    if (m_d->showColoring && m_d->coloringProjection) {
        rc |= m_d->coloringProjection->extent();
    }

    if (m_d->showKeyStrokes) {
        Q_FOREACH (const KeyStroke &stroke, m_d->keyStrokes) {
            rc |= stroke.dev->extent();
        }

        lockTemporaryTarget();
        KisPaintDeviceSP temporaryTarget = this->temporaryTarget();
        if (temporaryTarget) {
            rc |= temporaryTarget->extent();
        }
        unlockTemporaryTarget();
    }

    return rc;
}

QRect KisColorizeMask::exactBounds() const
{
    QRect rc;

    if (m_d->showColoring && m_d->coloringProjection) {
        rc |= m_d->coloringProjection->exactBounds();
    }

    if (m_d->showKeyStrokes) {
        Q_FOREACH (const KeyStroke &stroke, m_d->keyStrokes) {
            rc |= stroke.dev->exactBounds();
        }

        lockTemporaryTarget();
        KisPaintDeviceSP temporaryTarget = this->temporaryTarget();
        if (temporaryTarget) {
            rc |= temporaryTarget->exactBounds();
        }
        unlockTemporaryTarget();
    }

    return rc;

}

KisImageSP KisColorizeMask::fetchImage() const
{
    KisLayerSP parentLayer = dynamic_cast<KisLayer*>(parent().data());
    if (!parentLayer) return 0;

    return parentLayer->image();
}

void KisColorizeMask::setImage(KisImageWSP image)
{
    auto it = m_d->keyStrokes.begin();
    for(; it != m_d->keyStrokes.end(); ++it) {
        it->dev->setDefaultBounds(new KisDefaultBounds(image));
    }
}

void KisColorizeMask::setCurrentColor(const KoColor &color)
{
    lockTemporaryTargetForWrite();

    setNeedsUpdate(true);

    QList<KeyStroke>::const_iterator it =
        std::find_if(m_d->keyStrokes.constBegin(),
                     m_d->keyStrokes.constEnd(),
                     [color] (const KeyStroke &s) {
                         return s.color == color;
                     });

    KisPaintDeviceSP activeDevice;
    bool newKeyStroke = false;

    if (it == m_d->keyStrokes.constEnd()) {
        activeDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
        activeDevice->setParentNode(this);
        activeDevice->setDefaultBounds(new KisDefaultBounds(fetchImage()));
        newKeyStroke = true;
    } else {
        activeDevice = it->dev;
    }

    m_d->currentColor = color;
    m_d->currentKeyStrokeDevice = activeDevice;
    m_d->needAddCurrentKeyStroke = newKeyStroke;
    m_d->fakePaintDevice->convertTo(colorSpace());

    unlockTemporaryTarget();
}



struct KeyStrokeAddRemoveCommand : public KisCommandUtils::FlipFlopCommand {
    KeyStrokeAddRemoveCommand(bool add, int index, KeyStroke stroke, QList<KeyStroke> *list, KisNodeSP node)
        : FlipFlopCommand(!add),
          m_index(index), m_stroke(stroke),
          m_list(list), m_node(node) {}

    void init() {
        m_list->insert(m_index, m_stroke);
    }

    void end() {
        KIS_ASSERT_RECOVER_RETURN((*m_list)[m_index] == m_stroke);
        m_list->removeAt(m_index);
    }

private:
    int m_index;
    KeyStroke m_stroke;
    QList<KeyStroke> *m_list;
    // just a pointer to keep the node from deleting
    KisNodeSP m_node;
};

void KisColorizeMask::mergeToLayer(KisNodeSP layer, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText,int timedID)
{
    Q_UNUSED(layer);

    KisPaintDeviceSP temporaryTarget = this->temporaryTarget();
    const bool isTemporaryTargetErasing = temporaryCompositeOp() == COMPOSITE_ERASE;
    const QRect temporaryExtent = temporaryTarget ? temporaryTarget->extent() : QRect();

    KisSavedMacroCommand *macro = undoAdapter->createMacro(transactionText);
    KisMacroBasedUndoStore store(macro);
    KisPostExecutionUndoAdapter fakeUndoAdapter(&store, undoAdapter->strokesFacade());

    /**
     * Add a new key stroke plane
     */
    if (m_d->needAddCurrentKeyStroke && !isTemporaryTargetErasing) {
        KeyStroke key(m_d->currentKeyStrokeDevice, m_d->currentColor);

        KUndo2Command *cmd =
            new KeyStrokeAddRemoveCommand(
                true, m_d->keyStrokes.size(), key, &m_d->keyStrokes, layer);
        cmd->redo();
        fakeUndoAdapter.addCommand(toQShared(cmd));
    }

    /**
     * When erasing, the brush affects all the key strokes, not only
     * the current one.
     */
    if (!isTemporaryTargetErasing) {
        mergeToLayerImpl(m_d->currentKeyStrokeDevice, &fakeUndoAdapter, transactionText, timedID, false);
    } else {
        Q_FOREACH (const KeyStroke &stroke, m_d->keyStrokes) {
            if (temporaryExtent.intersects(stroke.dev->extent())) {
                mergeToLayerImpl(stroke.dev, &fakeUndoAdapter, transactionText, timedID, false);
            }
        }
    }

    mergeToLayerImpl(m_d->fakePaintDevice, &fakeUndoAdapter, transactionText, timedID, false);

    m_d->currentKeyStrokeDevice = 0;
    m_d->currentColor = KoColor();
    releaseResources();

    /**
     * Try removing the key strokes that has been completely erased
     */
    if (isTemporaryTargetErasing) {
        lockTemporaryTargetForWrite();

        int index = 0;
        auto it = m_d->keyStrokes.begin();
        while (it != m_d->keyStrokes.end()) {
            if (it->dev->exactBounds().isEmpty()) {
                fakeUndoAdapter.addCommand(
                    toQShared(
                        new KeyStrokeAddRemoveCommand(
                            false, index, *it, &m_d->keyStrokes, layer)));
                it = m_d->keyStrokes.erase(it);
            } else {
                ++it;
                ++index;
            }
        }

        unlockTemporaryTarget();
    }

    undoAdapter->addMacro(macro);
}

void KisColorizeMask::writeMergeData(KisPainter *painter, KisPaintDeviceSP src)
{
    const KoColorSpace *alpha8 = KoColorSpaceRegistry::instance()->alpha8();
    const bool nonAlphaDst = !(*painter->device()->colorSpace() == *alpha8);

    if (nonAlphaDst) {
        Q_FOREACH (const QRect &rc, src->region().rects()) {
            painter->bitBlt(rc.topLeft(), src, rc);
        }
    } else {
        KisSelectionSP conversionSelection = m_d->cachedConversionSelection.getSelection();
        KisPixelSelectionSP tempSelection = conversionSelection->pixelSelection();

        Q_FOREACH (const QRect &rc, src->region().rects()) {
            tempSelection->copyAlphaFrom(src, rc);
            painter->bitBlt(rc.topLeft(), tempSelection, rc);
        }
        m_d->cachedSelection.putSelection(conversionSelection);
    }
}

bool KisColorizeMask::showColoring() const
{
    return m_d->showColoring;
}

void KisColorizeMask::setShowColoring(bool value)
{
    QRect savedExtent;
    if (m_d->showColoring && !value) {
        savedExtent = extent();
    }

    m_d->showColoring = value;

    if (!savedExtent.isEmpty()) {
        setDirty(savedExtent);
    }
}

bool KisColorizeMask::showKeyStrokes() const
{
    return m_d->showKeyStrokes;
}

void KisColorizeMask::setShowKeyStrokes(bool value)
{
    QRect savedExtent;
    if (m_d->showKeyStrokes && !value) {
        savedExtent = extent();
    }

    m_d->showKeyStrokes = value;

    if (!savedExtent.isEmpty()) {
        setDirty(savedExtent);
    }
}

