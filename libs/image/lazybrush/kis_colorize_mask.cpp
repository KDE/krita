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

#include "kis_colorize_stroke_strategy.h"
#include "kis_multiway_cut.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_macro_based_undo_store.h"
#include "kis_post_execution_undo_adapter.h"
#include "kis_command_utils.h"
#include "kis_processing_applicator.h"
#include "krita_utils.h"
#include "kis_command_utils.h"


using namespace KisLazyFillTools;

struct KisColorizeMask::Private
{
    Private()
        : coloringProjection(new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8())),
          fakePaintDevice(new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8())),
          filteredSource(new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8())),
          needAddCurrentKeyStroke(false),
          showKeyStrokes(true),
          showColoring(true),
          needsUpdate(true),
          originalSequenceNumber(-1),
          updateCompressor(1, KisSignalCompressor::POSTPONE)
    {
    }

    Private(const Private &rhs)
        : coloringProjection(new KisPaintDevice(*rhs.coloringProjection)),
          fakePaintDevice(new KisPaintDevice(*rhs.fakePaintDevice)),
          filteredSource(new KisPaintDevice(*rhs.filteredSource)),
          needAddCurrentKeyStroke(rhs.needAddCurrentKeyStroke),
          showKeyStrokes(rhs.showKeyStrokes),
          showColoring(rhs.showColoring),
          needsUpdate(false),
          originalSequenceNumber(-1),
          updateCompressor(1000, KisSignalCompressor::POSTPONE),
          offset(rhs.offset)
    {
        Q_FOREACH (const KeyStroke &stroke, rhs.keyStrokes) {
            keyStrokes << KeyStroke(KisPaintDeviceSP(new KisPaintDevice(*stroke.dev)), stroke.color, stroke.isTransparent);
        }
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
    int originalSequenceNumber;

    KisSignalCompressor updateCompressor;
    QPoint offset;
};

KisColorizeMask::KisColorizeMask()
    : m_d(new Private)
{
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

void KisColorizeMask::initializeCompositeOp()
{
    KisLayerSP parentLayer(dynamic_cast<KisLayer*>(parent().data()));
    if (!parentLayer || !parentLayer->original()) return;

    KisImageSP image = parentLayer->image();
    if (!image) return;

    const qreal samplePortion = 0.1;
    const qreal alphaPortion =
        KritaUtils::estimatePortionOfTransparentPixels(parentLayer->original(),
                                                       image->bounds(),
                                                       samplePortion);

    setCompositeOpId(alphaPortion > 0.3 ? COMPOSITE_BEHIND : COMPOSITE_MULT);
}

const KoColorSpace* KisColorizeMask::colorSpace() const
{
    return m_d->fakePaintDevice->colorSpace();
}

struct SetKeyStrokesColorSpaceCommand : public KUndo2Command {
    SetKeyStrokesColorSpaceCommand(const KoColorSpace *dstCS,
                                   KoColorConversionTransformation::Intent renderingIntent,
                                   KoColorConversionTransformation::ConversionFlags conversionFlags,
                                   QList<KeyStroke> *list,
                                   KisColorizeMaskSP node)
        : m_dstCS(dstCS),
          m_renderingIntent(renderingIntent),
          m_conversionFlags(conversionFlags),
          m_list(list),
          m_node(node) {}

    void undo() override {
        KIS_ASSERT_RECOVER_RETURN(m_list->size() == m_oldColors.size());

        for (int i = 0; i < m_list->size(); i++) {
            (*m_list)[i].color = m_oldColors[i];
        }
    }

    void redo() override {
        if (m_oldColors.isEmpty()) {
            Q_FOREACH(const KeyStroke &stroke, *m_list) {
                m_oldColors << stroke.color;
                m_newColors << stroke.color;
                m_newColors.last().convertTo(m_dstCS, m_renderingIntent, m_conversionFlags);
            }
        }

        KIS_ASSERT_RECOVER_RETURN(m_list->size() == m_newColors.size());

        for (int i = 0; i < m_list->size(); i++) {
            (*m_list)[i].color = m_newColors[i];
        }
    }

private:
    QVector<KoColor> m_oldColors;
    QVector<KoColor> m_newColors;

    const KoColorSpace *m_dstCS;
    KoColorConversionTransformation::Intent m_renderingIntent;
    KoColorConversionTransformation::ConversionFlags m_conversionFlags;
    QList<KeyStroke> *m_list;
    KisColorizeMaskSP m_node;
};


void KisColorizeMask::setProfile(const KoColorProfile *profile)
{
    // WARNING: there is no undo information, used only while loading!

    m_d->fakePaintDevice->setProfile(profile);
    m_d->coloringProjection->setProfile(profile);

    for (auto stroke : m_d->keyStrokes) {
        stroke.color.setProfile(profile);
    }
}

KUndo2Command* KisColorizeMask::setColorSpace(const KoColorSpace * dstColorSpace,
                                              KoColorConversionTransformation::Intent renderingIntent,
                                              KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    using namespace KisCommandUtils;

    CompositeCommand *composite = new CompositeCommand();

    composite->addCommand(m_d->fakePaintDevice->convertTo(dstColorSpace, renderingIntent, conversionFlags));
    composite->addCommand(m_d->coloringProjection->convertTo(dstColorSpace, renderingIntent, conversionFlags));

    KUndo2Command *strokesConversionCommand =
        new SetKeyStrokesColorSpaceCommand(
            dstColorSpace, renderingIntent, conversionFlags,
            &m_d->keyStrokes, KisColorizeMaskSP(this));
    strokesConversionCommand->redo();

    composite->addCommand(new SkipFirstRedoWrapper(strokesConversionCommand));

    return composite;
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

    bool filteredSourceValid = m_d->originalSequenceNumber == src->sequenceNumber();
    m_d->originalSequenceNumber = src->sequenceNumber();
    m_d->coloringProjection->clear();

    KisLayerSP parentLayer(dynamic_cast<KisLayer*>(parent().data()));
    if (!parentLayer) return;

    KisImageSP image = parentLayer->image();
    if (image) {
        KisColorizeStrokeStrategy *strategy =
            new KisColorizeStrokeStrategy(src,
                                          m_d->coloringProjection,
                                          m_d->filteredSource,
                                          filteredSourceValid,
                                          image->bounds(),
                                          KisColorizeMaskSP(this));

        Q_FOREACH (const KeyStroke &stroke, m_d->keyStrokes) {
            const KoColor color =
                !stroke.isTransparent ?
                stroke.color :
                KoColor(Qt::transparent, stroke.color.colorSpace());

            strategy->addKeyStroke(stroke.dev, color);
        }

        connect(strategy, SIGNAL(sigFinished()), SLOT(slotRegenerationFinished()));
        KisStrokeId id = image->startStroke(strategy);
        image->endStroke(id);
    }
}

void KisColorizeMask::slotRegenerationFinished()
{
    setNeedsUpdate(true);
}

KisBaseNode::PropertyList KisColorizeMask::sectionModelProperties() const
{
    KisBaseNode::PropertyList l = KisMask::sectionModelProperties();
    l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::colorizeNeedsUpdate, needsUpdate());
    l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::colorizeEditKeyStrokes, showKeyStrokes());
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
        if (property.id == KisLayerPropertiesIcons::colorizeEditKeyStrokes.id()) {
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
    return m_d->showKeyStrokes ? m_d->fakePaintDevice : KisPaintDeviceSP();
}

KisPaintDeviceSP KisColorizeMask::coloringProjection() const
{
    return m_d->coloringProjection;
}

QIcon KisColorizeMask::icon() const
{
    return KisIconUtils::loadIcon("colorizeMask");
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

        if (m_d->showColoring && m_d->coloringProjection) {
            gc.setOpacity(opacity());
            gc.setCompositeOp(compositeOpId());
            gc.bitBlt(rect.topLeft(), m_d->coloringProjection, rect);
        }
    }

    // Draw the key strokes
    if (m_d->showKeyStrokes) {
        KisIndirectPaintingSupport::ReadLocker locker(this);

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

        KisIndirectPaintingSupport::ReadLocker locker(this);

        KisPaintDeviceSP temporaryTarget = this->temporaryTarget();
        if (temporaryTarget) {
            rc |= temporaryTarget->extent();
        }
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

        KisIndirectPaintingSupport::ReadLocker locker(this);
        KisPaintDeviceSP temporaryTarget = this->temporaryTarget();
        if (temporaryTarget) {
            rc |= temporaryTarget->exactBounds();
        }
    }

    return rc;

}

QRect KisColorizeMask::nonDependentExtent() const
{
    return extent();
}

KisImageSP KisColorizeMask::fetchImage() const
{
    KisLayerSP parentLayer(dynamic_cast<KisLayer*>(parent().data()));
    if (!parentLayer) return KisImageSP();

    return parentLayer->image();
}

void KisColorizeMask::setImage(KisImageWSP image)
{
    KisDefaultBoundsSP bounds(new KisDefaultBounds(image));

    auto it = m_d->keyStrokes.begin();
    for(; it != m_d->keyStrokes.end(); ++it) {
        it->dev->setDefaultBounds(bounds);
    }

    m_d->coloringProjection->setDefaultBounds(bounds);
    m_d->fakePaintDevice->setDefaultBounds(bounds);
    m_d->filteredSource->setDefaultBounds(bounds);
}

void KisColorizeMask::setCurrentColor(const KoColor &_color)
{
    KoColor color = _color;
    color.convertTo(colorSpace());

    WriteLocker locker(this);

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
        activeDevice->setDefaultBounds(KisDefaultBoundsBaseSP(new KisDefaultBounds(fetchImage())));
        newKeyStroke = true;
    } else {
        activeDevice = it->dev;
    }

    m_d->currentColor = color;
    m_d->currentKeyStrokeDevice = activeDevice;
    m_d->needAddCurrentKeyStroke = newKeyStroke;
}



struct KeyStrokeAddRemoveCommand : public KisCommandUtils::FlipFlopCommand {
    KeyStrokeAddRemoveCommand(bool add, int index, KeyStroke stroke, QList<KeyStroke> *list, KisColorizeMaskSP node)
        : FlipFlopCommand(!add),
          m_index(index), m_stroke(stroke),
          m_list(list), m_node(node) {}

    void init() override {
        m_list->insert(m_index, m_stroke);
        emit m_node->sigKeyStrokesListChanged();
    }

    void end() override {
        KIS_ASSERT_RECOVER_RETURN((*m_list)[m_index] == m_stroke);
        m_list->removeAt(m_index);
        emit m_node->sigKeyStrokesListChanged();
    }

private:
    int m_index;
    KeyStroke m_stroke;
    QList<KeyStroke> *m_list;
    KisColorizeMaskSP m_node;
};

void KisColorizeMask::mergeToLayer(KisNodeSP layer, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText,int timedID)
{
    Q_UNUSED(layer);

    WriteLocker locker(this);

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
                true, m_d->keyStrokes.size(), key, &m_d->keyStrokes, KisColorizeMaskSP(this));
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
        for (int index = 0; index < m_d->keyStrokes.size(); /*noop*/) {
            const KeyStroke &stroke = m_d->keyStrokes[index];

            if (stroke.dev->exactBounds().isEmpty()) {
                KUndo2Command *cmd =
                    new KeyStrokeAddRemoveCommand(
                        false, index, stroke, &m_d->keyStrokes, KisColorizeMaskSP(this));

                cmd->redo();
                fakeUndoAdapter.addCommand(toQShared(cmd));

            } else {
                index++;
            }
        }
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

KisColorizeMask::KeyStrokeColors KisColorizeMask::keyStrokesColors() const
{
    KeyStrokeColors colors;

    // TODO: thread safety!
    for (int i = 0; i < m_d->keyStrokes.size(); i++) {
        colors.colors << m_d->keyStrokes[i].color;

        if (m_d->keyStrokes[i].isTransparent) {
            colors.transparentIndex = i;
        }
    }

    return colors;
}

struct SetKeyStrokeColorsCommand : public KUndo2Command {
    SetKeyStrokeColorsCommand(const QList<KeyStroke> newList, QList<KeyStroke> *list, KisColorizeMaskSP node)
        : m_newList(newList),
          m_oldList(*list),
          m_list(list),
          m_node(node) {}

    void redo() override {
        *m_list = m_newList;

        emit m_node->sigKeyStrokesListChanged();
        m_node->setDirty();
    }

    void undo() override {
        *m_list = m_oldList;

        emit m_node->sigKeyStrokesListChanged();
        m_node->setDirty();
    }

private:
    QList<KeyStroke> m_newList;
    QList<KeyStroke> m_oldList;
    QList<KeyStroke> *m_list;
    KisColorizeMaskSP m_node;
};

void KisColorizeMask::setKeyStrokesColors(KeyStrokeColors colors)
{
    KIS_ASSERT_RECOVER_RETURN(colors.colors.size() == m_d->keyStrokes.size());

    QList<KeyStroke> newList = m_d->keyStrokes;

    for (int i = 0; i < newList.size(); i++) {
        newList[i].color = colors.colors[i];
        newList[i].color.convertTo(colorSpace());
        newList[i].isTransparent = colors.transparentIndex == i;
    }

    KisProcessingApplicator applicator(fetchImage(), KisNodeSP(this),
                                       KisProcessingApplicator::NONE,
                                       KisImageSignalVector(),
                                       kundo2_i18n("Change Key Stroke Color"));
    applicator.applyCommand(
        new SetKeyStrokeColorsCommand(
            newList, &m_d->keyStrokes, KisColorizeMaskSP(this)));

    applicator.end();
}

void KisColorizeMask::removeKeyStroke(const KoColor &_color)
{
    KoColor color = _color;
    color.convertTo(colorSpace());

    QList<KeyStroke>::iterator it =
        std::find_if(m_d->keyStrokes.begin(),
                     m_d->keyStrokes.end(),
                     [color] (const KeyStroke &s) {
                         return s.color == color;
                     });

    KIS_SAFE_ASSERT_RECOVER_RETURN(it != m_d->keyStrokes.end());

    const int index = it - m_d->keyStrokes.begin();

    KisProcessingApplicator applicator(KisImageWSP(fetchImage()), KisNodeSP(this),
                                       KisProcessingApplicator::NONE,
                                       KisImageSignalVector(),
                                       kundo2_i18n("Remove Key Stroke"));
    applicator.applyCommand(
        new KeyStrokeAddRemoveCommand(
            false, index, *it, &m_d->keyStrokes, KisColorizeMaskSP(this)));

    applicator.end();
}


QVector<KisPaintDeviceSP> KisColorizeMask::allPaintDevices() const
{
    QVector<KisPaintDeviceSP> devices;

    Q_FOREACH (const KeyStroke &stroke, m_d->keyStrokes) {
        devices << stroke.dev;
    }

    devices << m_d->coloringProjection;
    devices << m_d->fakePaintDevice;

    return devices;
}

void KisColorizeMask::resetCache()
{
    m_d->filteredSource->clear();
    m_d->originalSequenceNumber = -1;

    rerenderFakePaintDevice();
}


void KisColorizeMask::rerenderFakePaintDevice()
{
    m_d->fakePaintDevice->clear();
    KisFillPainter gc(m_d->fakePaintDevice);

    KisSelectionSP selection = m_d->cachedSelection.getSelection();

    Q_FOREACH (const KeyStroke &stroke, m_d->keyStrokes) {
        const QRect rect = stroke.dev->extent();

        selection->pixelSelection()->makeCloneFromRough(stroke.dev, rect);
        gc.setSelection(selection);
        gc.fillSelection(rect, stroke.color);
    }

    m_d->cachedSelection.putSelection(selection);
}

void KisColorizeMask::testingAddKeyStroke(KisPaintDeviceSP dev, const KoColor &color, bool isTransparent)
{
    m_d->keyStrokes << KeyStroke(dev, color, isTransparent);
}

void KisColorizeMask::testingRegenerateMask()
{
    slotUpdateRegenerateFilling();
}

KisPaintDeviceSP KisColorizeMask::testingFilteredSource() const
{
    return m_d->filteredSource;
}

QList<KeyStroke> KisColorizeMask::fetchKeyStrokesDirect() const
{
    return m_d->keyStrokes;
}

void KisColorizeMask::setKeyStrokesDirect(const QList<KisLazyFillTools::KeyStroke> &strokes)
{
    m_d->keyStrokes = strokes;

    KisImageSP image = fetchImage();
    KIS_SAFE_ASSERT_RECOVER_RETURN(image);
    setImage(image);
}

qint32 KisColorizeMask::x() const
{
    return m_d->offset.x();
}

qint32 KisColorizeMask::y() const
{
    return m_d->offset.y();
}

void KisColorizeMask::setX(qint32 x)
{
    const QPoint oldOffset = m_d->offset;
    m_d->offset.rx() = x;
    moveAllInternalDevices(m_d->offset - oldOffset);
}

void KisColorizeMask::setY(qint32 y)
{
    const QPoint oldOffset = m_d->offset;
    m_d->offset.ry() = y;
    moveAllInternalDevices(m_d->offset - oldOffset);
}

KisPaintDeviceList KisColorizeMask::getLodCapableDevices() const
{
    KisPaintDeviceList list;

    auto it = m_d->keyStrokes.begin();
    for(; it != m_d->keyStrokes.end(); ++it) {
        list << it->dev;
    }

    list << m_d->coloringProjection;
    list << m_d->fakePaintDevice;
    list << m_d->filteredSource;

    return list;
}

void KisColorizeMask::moveAllInternalDevices(const QPoint &diff)
{
    QVector<KisPaintDeviceSP> devices = allPaintDevices();

    Q_FOREACH (KisPaintDeviceSP dev, devices) {
        dev->moveTo(dev->offset() + diff);
    }
}
