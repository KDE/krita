/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "SvgTextCursor.h"
#include "KoCanvasBase.h"
#include "KoSvgTextProperties.h"
#include "SvgTextInsertCommand.h"
#include "SvgTextInsertRichCommand.h"
#include "SvgTextMergePropertiesRangeCommand.h"
#include "SvgTextRemoveCommand.h"
#include "SvgTextRemoveTransformsFromRange.h"
#include "SvgTextShapeManagerBlocker.h"
#include "SvgTextShortCuts.h"

#include "KoSvgTextShapeMarkupConverter.h"
#include "KoSvgPaste.h"
#include "KoColorBackground.h"
#include "KoShapeStroke.h"
#include "KoColor.h"

#include "KoViewConverter.h"
#include "kis_coordinates_converter.h"
#include "kis_painting_tweaks.h"
#include "KoCanvasController.h"
#include "KoCanvasResourceProvider.h"
#include <kis_signal_compressor.h>
#include <KisHandlePainterHelper.h>

#include "kundo2command.h"
#include <QTimer>
#include <QDebug>
#include <QClipboard>
#include <QMimeData>
#include <QApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QAction>
#include <kis_assert.h>
#include <QInputMethodEvent>
#include <QBuffer>
#include <QWidget>


struct IMEDecorationInfo {
    int start = -1; ///< The startPos from the attribute.
    int length = 0; ///< The length from the attribute.
    KoSvgText::TextDecorations decor = KoSvgText::DecorationNone; ///< Which sides get decorated.
    KoSvgText::TextDecorationStyle style = KoSvgText::Solid; ///< The style.
    bool thick = false; ///< Whether the decoration needs to be doubled in size.

    void setDecorationFromQStyle(QTextCharFormat::UnderlineStyle s) {
        // whenever qt sets an underlinestyle it always sets the underline.
        decor.setFlag(KoSvgText::DecorationUnderline, s != QTextCharFormat::NoUnderline);
        if (s == QTextCharFormat::DotLine) {
            style = KoSvgText::Dotted;
        } else if (s == QTextCharFormat::DashUnderline) {
            style = KoSvgText::Dashed;
        } else if (s == QTextCharFormat::WaveUnderline) {
            style = KoSvgText::Wavy;
        } else if (s == QTextCharFormat::SpellCheckUnderline) {
            style = KoSvgText::Wavy;
#ifdef Q_OS_MACOS
            style = KoSvgText::Dotted;
#endif
        } else {
            style = KoSvgText::Solid;
        }
    }

    void setDecorationFromQTextCharFormat(QTextCharFormat format) {
        if (format.hasProperty(QTextFormat::FontUnderline)) {
            decor.setFlag(KoSvgText::DecorationUnderline, format.property(QTextFormat::FontUnderline).toBool());
        }
        if (format.hasProperty(QTextFormat::FontOverline)) {
            decor.setFlag(KoSvgText::DecorationOverline, format.property(QTextFormat::FontOverline).toBool());
        }
        if (format.hasProperty(QTextFormat::FontStrikeOut)) {
            decor.setFlag(KoSvgText::DecorationLineThrough, format.property(QTextFormat::FontStrikeOut).toBool());
        }

        if (format.hasProperty(QTextFormat::TextUnderlineStyle)) {
            setDecorationFromQStyle(format.underlineStyle());
        }
        /**
         * Because Qt doesn't have a concept of a thick or double underline at time of writing,
         * most of Qt's QPA will set the background to a solid color instead. Sometimes the underline
         * style is changed (as with IBus). We don't support setting the background right now, so instead
         * we'll 'convert' it back to a thick solid underline.
         */
        if (format.hasProperty(QTextFormat::BackgroundBrush)) {
            thick = format.background().isOpaque();
#ifdef Q_OS_LINUX
            if (style == KoSvgText::Dashed) {
                style = KoSvgText::Solid;
            }
#endif

        }
        if (decor == KoSvgText::DecorationNone) {
            // Ensure a underline is always set.
            decor.setFlag(KoSvgText::DecorationUnderline, true);
        }
    }
};

struct TypeSettingDecorInfo {
    QPair<QPointF, QPointF> handles;
    bool handlesEnabled;

    QMap<SvgTextCursor::TypeSettingModeHandle, QPainterPath> baselines;
    QMap<SvgTextCursor::TypeSettingModeHandle, QPainterPath> paths;

    QPointF closestBaselinePoint;

    QRectF boundingRect(qreal handleRadius) {
        QRectF total;
        for (int i = 0; i< paths.values().size(); i++) {
            total |= paths.values().at(i).boundingRect();
        }
        for (int i = 0; i< baselines.values().size(); i++) {
            total |= baselines.values().at(i).boundingRect();
        }
        QRectF rect(0, 0, handleRadius, handleRadius);
        rect.moveCenter(handles.first);
        total |= rect;
        rect.moveCenter(handles.second);
        total |= rect;
        return total;
    }

    bool testBaselines(Qt::KeyboardModifiers modifiers) {
        return (modifiers & Qt::ShiftModifier);
    }
};

struct Q_DECL_HIDDEN SvgTextCursor::Private {
    KoCanvasBase *canvas;
    bool isAddingCommand = false;
    int pos = 0;
    int anchor = 0;
    KoSvgTextShape *shape {nullptr};

    QTimer cursorFlash;
    QTimer cursorFlashLimit;
    bool cursorVisible = false;

    QPainterPath cursorShape;
    QColor cursorColor;
    QRectF oldCursorRect;
    QLineF cursorCaret;
    QLineF anchorCaret;
    int cursorWidth = 1;
    bool drawCursorInAdditionToSelection = false;
    QPainterPath selection;
    QRectF oldSelectionRect;


    // This is used to adjust cursorpositions better on text-shape relayouts.
    int posIndex = 0;
    int anchorIndex = 0;

    bool visualNavigation = true;
    bool pasteRichText = true;

    Qt::KeyboardModifiers lastKnownModifiers;

    bool typeSettingMode = false;
    SvgTextCursor::TypeSettingModeHandle hoveredTypeSettingHandle = SvgTextCursor::NoHandle;
    bool drawTypeSettingHandle = true;
    qreal handleRadius = 7;
    QRectF oldTypeSettingRect;
    TypeSettingDecorInfo typeSettingDecor;

    SvgTextInsertCommand *preEditCommand {nullptr}; ///< PreEdit string as an command provided by the input method.
    int preEditStart = -1; ///< Start of the preEdit string as a cursor pos.
    int preEditLength = -1; ///< Length of the preEditString.
    QVector<IMEDecorationInfo> styleMap; ///< Decoration info (underlines) for the preEdit string to differentiate it from regular text.
    QPainterPath IMEDecoration; ///< The decorations for the current preedit string.
    QRectF oldIMEDecorationRect; ///< Update Rectangle of previous decoration.
    bool blockQueryUpdates = false; ///< Block qApp->inputMethod->update(), enabled during the inputmethod event flow.

    SvgTextCursorPropertyInterface *interface{nullptr};

    QList<QAction*> actions;
};

SvgTextCursor::SvgTextCursor(KoCanvasBase *canvas) :
    d(new Private)
{
    d->canvas = canvas;
    if (d->canvas->canvasController()) {
        // Mockcanvas in the tests has no canvas controller.
        connect(d->canvas->canvasController()->proxyObject, SIGNAL(sizeChanged(QSize)), this, SLOT(updateInputMethodItemTransform()));
        connect(d->canvas->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
                this, SLOT(canvasResourceChanged(int,QVariant)));
    }
    d->interface = new SvgTextCursorPropertyInterface(this);
}

SvgTextCursor::~SvgTextCursor()
{
    commitIMEPreEdit();
    d->cursorFlash.stop();
    d->cursorFlashLimit.stop();
    d->shape = nullptr;
}

KoSvgTextShape *SvgTextCursor::shape() const
{
    return d->shape;
}

void SvgTextCursor::setShape(KoSvgTextShape *textShape)
{
    if (d->shape) {
        commitIMEPreEdit();
        d->shape->removeShapeChangeListener(this);
    }
    d->shape = textShape;
    if (d->shape) {
        d->shape->addShapeChangeListener(this);
        updateInputMethodItemTransform();
        d->pos = d->shape->posForIndex(d->shape->plainText().size());
        updateTypeSettingDecorFromShape();
    } else {
        d->pos = 0;
    }
    d->anchor = 0;
    updateCursor(true);
    updateSelection();
    d->interface->emitSelectionChange();
}

void SvgTextCursor::setCaretSetting(int cursorWidth, int cursorFlash, int cursorFlashLimit, bool drawCursorInAdditionToSelection)
{
    d->cursorFlash.setInterval(cursorFlash/2);
    d->cursorFlashLimit.setInterval(cursorFlashLimit);
    d->cursorWidth = cursorWidth;
    d->drawCursorInAdditionToSelection = drawCursorInAdditionToSelection;
    connect(&d->cursorFlash, SIGNAL(timeout()), this, SLOT(blinkCursor()));
    connect(&d->cursorFlashLimit, SIGNAL(timeout()), this, SLOT(stopBlinkCursor()));
}

void SvgTextCursor::setVisualMode(bool visualMode)
{
    d->visualNavigation = visualMode;
}

void SvgTextCursor::setPasteRichTextByDefault(const bool pasteRichText)
{
    d->pasteRichText = pasteRichText;
}

void SvgTextCursor::setTypeSettingModeActive(bool activate)
{
    d->typeSettingMode = activate;
    updateTypeSettingDecoration();
}

int SvgTextCursor::getPos()
{
    return d->pos;
}

int SvgTextCursor::getAnchor()
{
    return d->anchor;
}

void SvgTextCursor::setPos(int pos, int anchor)
{
    d->pos = pos;
    d->anchor = anchor;
    updateCursor();
    updateSelection();
}

void SvgTextCursor::setPosToPoint(QPointF point, bool moveAnchor)
{
    if (d->shape) {
        int pos = d->shape->posForPointLineSensitive(d->shape->documentToShape(point));
        if (d->preEditCommand) {
            int start = d->shape->indexForPos(d->preEditStart);
            int end = start + d->preEditLength;
            int posIndex = d->shape->indexForPos(pos);
            if (posIndex > start && posIndex <= end) {
                qApp->inputMethod()->invokeAction(QInputMethod::Click, posIndex - start);
                return;
            } else {
                commitIMEPreEdit();
            }
        }

        const int finalPos = d->shape->posForIndex(d->shape->plainText().size());
        d->pos = qBound(0, pos, finalPos);
        if (moveAnchor || d->anchor < 0 || d->anchor > finalPos) {
            d->anchor = d->pos;
        }
        updateCursor();
        updateSelection();
    }
}

SvgTextCursor::TypeSettingModeHandle SvgTextCursor::typeSettingHandleAtPos(const QRectF regionOfInterest)
{
    SvgTextCursor::TypeSettingModeHandle handle = SvgTextCursor::NoHandle;

    if (!(d->typeSettingMode && d->shape && d->canvas)) return handle;

    const QRectF roiInShape = d->shape->absoluteTransformation().inverted().mapRect(regionOfInterest);

    if (d->typeSettingDecor.handlesEnabled) {
        if (roiInShape.contains(d->typeSettingDecor.handles.first)) {
            handle = SvgTextCursor::StartPos;
        } else if (roiInShape.contains(d->typeSettingDecor.handles.second)) {
            handle = SvgTextCursor::EndPos;
        }
    }
    if (handle != NoHandle) return handle;
    if (!d->typeSettingDecor.boundingRect(d->handleRadius).intersects(roiInShape)) return handle;

    qreal closest = std::numeric_limits<qreal>::max();
    QMap<SvgTextCursor::TypeSettingModeHandle, QPainterPath> paths
            = d->typeSettingDecor.testBaselines(d->lastKnownModifiers)? d->typeSettingDecor.baselines: d->typeSettingDecor.paths;
    Q_FOREACH (const SvgTextCursor::TypeSettingModeHandle baseline, paths.keys()) {
        const QPainterPath path = paths.value(baseline);
        if (!path.intersects(roiInShape)) continue;

        const QList<QPolygonF> polys = path.toSubpathPolygons();
        Q_FOREACH(const QPolygonF poly, polys) {
            if (poly.size() < 2) continue;
            for (int i = 1; i < poly.size(); i++) {
                QLineF l(poly.at(i-1), poly.at(i));
                qreal distance = kisDistanceToLine(roiInShape.center(), l);
                if (distance < closest) {
                    handle = baseline;
                    closest = distance;
                    d->typeSettingDecor.closestBaselinePoint =
                            kisProjectOnVector(l.p2() - l.p1(), roiInShape.center() - l.p1()) + l.p1();
                }
            }
        }
    }

    return handle;
}

void SvgTextCursor::setTypeSettingHandleHovered(TypeSettingModeHandle hovered)
{
    d->hoveredTypeSettingHandle = hovered;
}

void SvgTextCursor::setDrawTypeSettingHandle(bool draw)
{
    d->drawTypeSettingHandle = draw;
}

void SvgTextCursor::updateTypeSettingDecorFromShape()
{
    if (d->shape) {
        d->typeSettingDecor.handlesEnabled = (d->shape->textType() == KoSvgTextShape::PreformattedText
                                              || d->shape->textType() == KoSvgTextShape::PrePositionedText);
    }
}

QCursor SvgTextCursor::cursorTypeForTypeSetting() const
{
    if (d->hoveredTypeSettingHandle == StartPos ||
            d->hoveredTypeSettingHandle == StartPos ||
            d->typeSettingDecor.testBaselines(d->lastKnownModifiers)) {
        return Qt::ArrowCursor;
    } else if (d->shape) {
        return (d->shape->writingMode() == KoSvgText::HorizontalTB)? Qt::SizeVerCursor: Qt::SizeHorCursor;
    }
    return Qt::ArrowCursor;
}

QString SvgTextCursor::handleName(TypeSettingModeHandle handle) const
{
    bool baseline = d->typeSettingDecor.testBaselines(d->lastKnownModifiers);
    if (handle == Ascender) {
        if (baseline) {
            return i18nc("Type setting mode line name", "Text Top");
        } else {
            return i18nc("Type setting mode line name", "Font Size");
        }
    } else if (handle == Descender) {
        if (baseline) {
            return i18nc("Type setting mode line name", "Text Bottom");
        } else {
            return i18nc("Type setting mode line name", "Font Size");
        }
    } else if (handle == BaselineAlphabetic) {
        return i18nc("Type setting mode line name", "Alphabetic");
    } else if (handle == BaselineIdeographic) {
        return i18nc("Type setting mode line name", "Ideographic");
    } else if (handle == BaselineHanging) {
        return i18nc("Type setting mode line name", "Hanging");
    } else if (handle == BaselineMiddle) {
        return i18nc("Type setting mode line name", "Middle");
    } else if (handle == BaselineMathematical) {
        return i18nc("Type setting mode line name", "Mathematical");
    } else if (handle == BaselineCentral) {
        return i18nc("Type setting mode line name", "Central");
    } else if (handle == LineHeightTop || handle == LineHeightBottom) {
        return i18nc("Type setting mode line name", "Line Height");
    } else if (handle == BaselineShift) {
        if (baseline) {
            return i18nc("Type setting mode line name", "Current Baseline");
        } else {
            return i18nc("Type setting mode line name", "Baseline Shift");
        }
    } else {
        return QString();
    }
}

bool SvgTextCursor::setDominantBaselineFromHandle(const TypeSettingModeHandle handle)
{
    if (handle == NoHandle) return false;
    if (!d->typeSettingDecor.testBaselines(d->lastKnownModifiers)) return false;
    KoSvgText::Baseline baseline = KoSvgText::BaselineAuto;
    if (handle == Ascender) {
        baseline = KoSvgText::BaselineTextTop;
    } else if (handle == Descender) {
        baseline = KoSvgText::BaselineTextBottom;
    } else if (handle == BaselineAlphabetic) {
        baseline = KoSvgText::BaselineAlphabetic;
    } else if (handle == BaselineIdeographic) {
        baseline = KoSvgText::BaselineIdeographic;
    } else if (handle == BaselineHanging) {
        baseline = KoSvgText::BaselineHanging;
    } else if (handle == BaselineMiddle) {
        baseline = KoSvgText::BaselineMiddle;
    } else if (handle == BaselineMathematical) {
        baseline = KoSvgText::BaselineMathematical;
    } else if (handle == BaselineCentral) {
        baseline = KoSvgText::BaselineCentral;
    } else {
        return false;
    }
    KoSvgTextProperties props;
    props.setProperty(KoSvgTextProperties::DominantBaselineId, QVariant::fromValue(baseline));
    props.setProperty(KoSvgTextProperties::AlignmentBaselineId, QVariant::fromValue(baseline));
    mergePropertiesIntoSelection(props);
    return true;
}
// The baselines that need to get precendence over the others need to go later in the list.
QMap<SvgTextCursor::TypeSettingModeHandle, int> typeSettingBaselinesFromMetrics(const KoSvgText::FontMetrics metrics, const qreal lineGap, const bool isHorizontal) {
    return QMap<SvgTextCursor::TypeSettingModeHandle, int> {
        {SvgTextCursor::Ascender, metrics.ascender},
        {SvgTextCursor::Descender, metrics.descender},
        {SvgTextCursor::BaselineAlphabetic, metrics.alphabeticBaseline},
        {SvgTextCursor::BaselineIdeographic, metrics.ideographicUnderBaseline},
        {SvgTextCursor::BaselineHanging, metrics.hangingBaseline},
        {SvgTextCursor::BaselineCentral, metrics.ideographicCenterBaseline},
        {SvgTextCursor::BaselineMathematical, metrics.mathematicalBaseline},
        {SvgTextCursor::BaselineMiddle, isHorizontal? metrics.xHeight/2: metrics.ideographicCenterBaseline},
        {SvgTextCursor::LineHeightTop, metrics.ascender+(lineGap/2)},
        {SvgTextCursor::LineHeightBottom, metrics.descender-(lineGap/2)},
        {SvgTextCursor::BaselineShift, 0}
    };
}

int SvgTextCursor::posForTypeSettingHandleAndRect(const TypeSettingModeHandle handle, const QRectF regionOfInterest)
{
    if (!d->shape) return 0;

    QList<KoSvgTextCharacterInfo> infos =
            d->shape->getPositionsAndRotationsForRange(d->pos, d->anchor);
    if (infos.size() < 1) return 0;

    const QRectF roi = d->shape->documentToShape(regionOfInterest);

    for (auto it = infos.begin(); it != infos.end(); it++) {
        const int currentPos = (d->pos == d->anchor)? -1 :d->shape->posForIndex(it->logicalIndex);
        const KoSvgTextProperties props = d->shape->propertiesForPos(currentPos, true);
        KoSvgText::LineHeightInfo lineHeight = props.propertyOrDefault(KoSvgTextProperties::LineHeightId).value<KoSvgText::LineHeightInfo>();

        KoSvgText::FontMetrics metrics = (d->pos == d->anchor)? props.metrics(true, true): it->metrics;
        const bool isHorizontal = d->shape->writingMode() == KoSvgText::HorizontalTB;

        const qreal scaleMetrics = props.fontSize().value/qreal(metrics.fontSize);
        const int lineGap = lineHeight.isNormal? metrics.lineGap: (lineHeight.length.value/scaleMetrics)-(metrics.ascender-metrics.descender);

        QTransform t = QTransform::fromTranslate(it->finalPos.x(), it->finalPos.y());
        t.rotate(it->rotateDeg);

        const QMap<SvgTextCursor::TypeSettingModeHandle, int> types
                = typeSettingBaselinesFromMetrics(metrics, lineGap, isHorizontal);

        const int metric = types.value(handle);
        QPointF offset = isHorizontal? QPointF(0, -(metric*scaleMetrics)): QPointF(metric*scaleMetrics, 0);
        QLineF line = t.map(QLineF(offset, offset+it->advance));
        if (KisAlgebra2D::intersectLineRect(line, roi.toAlignedRect(), false)) return d->shape->posForIndex(it->logicalIndex);
    }

    return 0;
}



void SvgTextCursor::moveCursor(MoveMode mode, bool moveAnchor)
{
    if (d->shape) {

        const int finalPos = d->shape->posForIndex(d->shape->plainText().size());
        d->pos = qBound(0, moveModeResult(mode, d->pos, d->visualNavigation), finalPos);

        if (moveAnchor) {
            d->anchor = d->pos;
        }
        updateSelection();
        updateCursor();
    }
}

void SvgTextCursor::insertText(QString text)
{

    if (d->shape) {
        //KUndo2Command *parentCmd = new KUndo2Command;
        if (hasSelection()) {
            SvgTextRemoveCommand *removeCmd = removeSelectionImpl(false);
            addCommandToUndoAdapter(removeCmd);
        }

        SvgTextInsertCommand *insertCmd = new SvgTextInsertCommand(d->shape, d->pos, d->anchor, text);
        addCommandToUndoAdapter(insertCmd);

    }
}

void SvgTextCursor::insertRichText(KoSvgTextShape *insert)
{
    if (d->shape) {
        //KUndo2Command *parentCmd = new KUndo2Command;
        if (hasSelection()) {
            SvgTextRemoveCommand *removeCmd = removeSelectionImpl(false);
            addCommandToUndoAdapter(removeCmd);
        }

        SvgTextInsertRichCommand *cmd = new SvgTextInsertRichCommand(d->shape, insert, d->pos, d->anchor);
        addCommandToUndoAdapter(cmd);

    }
}

void SvgTextCursor::removeText(SvgTextCursor::MoveMode first, SvgTextCursor::MoveMode second)
{
    if (d->shape) {
        SvgTextRemoveCommand *removeCmd;
        if (hasSelection()) {
            removeCmd = removeSelectionImpl(true);
            addCommandToUndoAdapter(removeCmd);
        } else {
            int posA = moveModeResult(first, d->pos, d->visualNavigation);
            int posB = moveModeResult(second, d->pos, d->visualNavigation);

            int posStart = qMin(posA, posB);
            int posEnd = qMax(posA, posB);
            int indexEnd = d->shape->indexForPos(posEnd);
            int length = indexEnd - d->shape->indexForPos(posStart);

            removeCmd = new SvgTextRemoveCommand(d->shape, indexEnd, d->pos, d->anchor, length, true);
            addCommandToUndoAdapter(removeCmd);
        }
    }
}

void SvgTextCursor::removeLastCodePoint()
{
    if (d->shape) {
        SvgTextRemoveCommand *removeCmd;
        if (hasSelection()) {
            removeCmd = removeSelectionImpl(true);
            addCommandToUndoAdapter(removeCmd);
        } else {
            int lastIndex = d->shape->indexForPos(d->pos);
            removeCmd = new SvgTextRemoveCommand(d->shape, lastIndex, d->pos, d->anchor, 1, true);
            addCommandToUndoAdapter(removeCmd);
        }
    }
}

QPair<KoSvgTextProperties, KoSvgTextProperties> SvgTextCursor::currentTextProperties() const
{
    if (d->shape) {
        return QPair<KoSvgTextProperties, KoSvgTextProperties>(d->shape->propertiesForPos(d->pos), d->shape->propertiesForPos(d->pos, true));
    }
    return QPair<KoSvgTextProperties, KoSvgTextProperties>();
}

QList<KoSvgTextProperties> SvgTextCursor::propertiesForRange() const
{
    if (!d->shape) return QList<KoSvgTextProperties>();
    int start = -1;
    int end = -1;
    start = qMin(d->pos, d->anchor);
    end = qMax(d->pos, d->anchor);
    return d->shape->propertiesForRange(start, end);
}

QList<KoSvgTextProperties> SvgTextCursor::propertiesForShape() const
{
    if (!d->shape) return QList<KoSvgTextProperties>();
    return {d->shape->propertiesForRange(-1, -1)};
}

void SvgTextCursor::mergePropertiesIntoSelection(const KoSvgTextProperties props, const QSet<KoSvgTextProperties::PropertyId> removeProperties, bool paragraphOnly, bool selectWord)
{
    if (d->shape) {
        int start = -1;
        int end = -1;
        if (!paragraphOnly) {
            start = d->pos;
            end = d->anchor;
        }
        if (selectWord && d->pos == d->anchor) {
            const int finalPos = d->shape->posForIndex(d->shape->plainText().size());
            start = qBound(0, moveModeResult(MoveWordStart, d->pos, d->visualNavigation), finalPos);
            end = qBound(0, moveModeResult(MoveWordEnd, d->pos, d->visualNavigation), finalPos);
        }
        KUndo2Command *cmd = new SvgTextMergePropertiesRangeCommand(d->shape, props, start, end, removeProperties);
        addCommandToUndoAdapter(cmd);
    }
}

void SvgTextCursor::removeSelection()
{
    KUndo2Command *removeCmd = removeSelectionImpl(true);
    addCommandToUndoAdapter(removeCmd);
}

SvgTextRemoveCommand *SvgTextCursor::removeSelectionImpl(bool allowCleanUp, KUndo2Command *parent)
{
    SvgTextRemoveCommand *removeCmd = nullptr;
    if (d->shape) {
        if (d->anchor != d->pos) {
            int end = d->shape->indexForPos(qMax(d->anchor, d->pos));
            int length = d->shape->indexForPos(qMax(d->anchor, d->pos)) - d->shape->indexForPos(qMin(d->anchor, d->pos));
            removeCmd = new SvgTextRemoveCommand(d->shape, end, d->pos, d->anchor, length, allowCleanUp, parent);
        }
    }
    return removeCmd;
}

void SvgTextCursor::copy() const
{
    if (d->shape) {
        int start = d->shape->indexForPos(qMin(d->anchor, d->pos));
        int length = d->shape->indexForPos(qMax(d->anchor, d->pos)) - start;
        QString copied = d->shape->plainText().mid(start, length);
        std::unique_ptr<KoSvgTextShape> copy = d->shape->copyRange(start, length);
        QClipboard *cb = QApplication::clipboard();

        if (copy) {
            KoSvgTextShapeMarkupConverter converter(copy.get());
            QString svg;
            QString styles;
            QString html;
            QMimeData *svgData = new QMimeData();
            if (converter.convertToSvg(&svg, &styles)) {
                QString svgDoc = QString("<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"2.0\">%1\n%2</svg>").arg(styles).arg(svg);
                svgData->setData(QLatin1String("image/svg+xml"), svgDoc.toUtf8());
            }
            svgData->setText(copied);
            if (converter.convertToHtml(&html))
                svgData->setHtml(html);
            cb->setMimeData(svgData);
        } else {
            cb->setText(copied);
        }

    }
}

bool SvgTextCursor::paste()
{
    bool success = d->pasteRichText? pasteRichText(): pastePlainText();
    return success;
}

bool SvgTextCursor::pasteRichText()
{
    bool success = false;
    if (d->shape) {
        QClipboard *cb = QApplication::clipboard();
        const QMimeData *mimeData = cb->mimeData();
        KoSvgPaste shapePaste;
        if (shapePaste.hasShapes()) {
            QList<KoShape*> shapes = shapePaste.fetchShapes(d->shape->boundingRect(), 72.0);
            while (shapes.size() > 0) {
                KoSvgTextShape *textShape = dynamic_cast<KoSvgTextShape*>(shapes.takeFirst());
                if (textShape) {
                    insertRichText(textShape);
                    success = true;
                }
            }
        } else if (mimeData->hasHtml()) {
            QString html = mimeData->html();
            KoSvgTextShape *insert = new KoSvgTextShape();
            KoSvgTextShapeMarkupConverter converter(insert);
            QString svg;
            QString styles;
            if (converter.convertFromHtml(html, &svg, &styles)
                    && converter.convertFromSvg(svg, styles, d->shape->boundingRect(), 72.0) ) {
                insertRichText(insert);
                success = true;
            }
        }

        if (!success) {
            success = pastePlainText();
        }
    }
    return success;
}

bool SvgTextCursor::pastePlainText()
{
    bool success = false;
    QClipboard *cb = QApplication::clipboard();
    const QMimeData *mimeData = cb->mimeData();
    if (mimeData->hasText()) {
        insertText(mimeData->text());
        success = true;
    }
    return success;
}

void SvgTextCursor::removeTransformsFromRange()
{
    if (d->shape) {
        KUndo2Command *cmd = new SvgTextRemoveTransformsFromRange(d->shape, d->pos, d->anchor);
        addCommandToUndoAdapter(cmd);
    }
}

void SvgTextCursor::deselectText()
{
    setPos(d->pos, d->pos);
}

static QColor bgColorForCaret(QColor c, int opacity = 64) {

    return KisPaintingTweaks::luminosityCoarse(c) > 0.8? QColor(0, 0, 0, opacity) : QColor(255, 255, 255, opacity);
}

void SvgTextCursor::paintDecorations(QPainter &gc, QColor selectionColor, int decorationThickness, qreal handleRadius)
{
    if (d->shape) {
        gc.save();
        gc.setTransform(d->shape->absoluteTransformation(), true);

        if (d->pos != d->anchor && !d->typeSettingMode) {
            gc.save();
            gc.setOpacity(0.5);
            QBrush brush(selectionColor);
            gc.fillPath(d->selection, brush);
            gc.restore();
        }

        if ( (d->drawCursorInAdditionToSelection || d->pos == d->anchor)
                && d->cursorVisible) {
            QPen pen;
            pen.setCosmetic(true);
            QColor c = d->cursorColor.isValid()? d->cursorColor: Qt::black;
            pen.setColor(bgColorForCaret(c));
            pen.setWidth((d->cursorWidth + 2) * decorationThickness);
            gc.setPen(pen);
            gc.drawPath(d->cursorShape);
            pen.setColor(c);
            pen.setWidth(d->cursorWidth * decorationThickness);
            gc.setPen(pen);
            gc.drawPath(d->cursorShape);

        }

        if (d->preEditCommand) {
            gc.save();
            QBrush brush(selectionColor);
            gc.setOpacity(0.5);
            gc.fillPath(d->IMEDecoration, brush);
            gc.restore();
        }
        if (d->typeSettingMode && d->drawTypeSettingHandle) {
            d->handleRadius = handleRadius;
            QTransform painterTf = gc.transform();
            KisHandlePainterHelper helper(&gc, handleRadius, decorationThickness);
            const KisHandleStyle highlight = KisHandleStyle::partiallyHighlightedPrimaryHandles();
            const KisHandleStyle regular = KisHandleStyle::secondarySelection();

            QMap<SvgTextCursor::TypeSettingModeHandle, QPainterPath> paths
                    = d->typeSettingDecor.testBaselines(d->lastKnownModifiers)? d->typeSettingDecor.baselines: d->typeSettingDecor.paths;
            Q_FOREACH(SvgTextCursor::TypeSettingModeHandle handle, paths.keys()) {
                const QPainterPath p = paths.value(handle);
                if (d->hoveredTypeSettingHandle == handle) {
                    helper.setHandleStyle(highlight);
                    helper.drawPath(p);


                } else {
                    gc.save();
                    QPen pen(selectionColor, decorationThickness, handle == BaselineShift? Qt::SolidLine: Qt::DashLine);
                    pen.setCosmetic(true);
                    gc.setPen(pen);
                    gc.setOpacity(0.5);
                    gc.drawPath(painterTf.map(p));
                    gc.restore();
                }

            }

            if (d->typeSettingDecor.handlesEnabled) {
                helper.setHandleStyle(d->hoveredTypeSettingHandle == EndPos? highlight: regular);
                helper.drawHandleCircle(d->typeSettingDecor.handles.second);
                helper.setHandleStyle(d->hoveredTypeSettingHandle == StartPos? highlight: regular);
                helper.drawHandleRect(d->typeSettingDecor.handles.first);
            }
            QString name = handleName(d->hoveredTypeSettingHandle);
            if (!name.isEmpty()) {
                QPainterPath textP;
                textP.addText(painterTf.map(d->typeSettingDecor.closestBaselinePoint), gc.font(), name);
                gc.save();
                QPen pen(bgColorForCaret(selectionColor, 255));
                pen.setCosmetic(true);
                pen.setWidth(decorationThickness);
                gc.setPen(pen);
                gc.drawPath(textP);
                gc.fillPath(textP, QBrush(selectionColor));
                gc.restore();
            }

        }
        gc.restore();
    }
}

QVariant SvgTextCursor::inputMethodQuery(Qt::InputMethodQuery query) const
{
    dbgTools << "receiving inputmethod query" << query;

    // Because we set the input item transform to be shape->document->view->widget->window,
    // the coordinates here should be in shape coordinates.
    switch(query) {
    case Qt::ImEnabled:
        return d->shape? true: false;
        break;
    case Qt::ImCursorRectangle:
        // The platform integration will always define the cursor as the 'left side' handle.
        if (d->shape) {
            QPointF caret1(d->cursorCaret.p1());
            QPointF caret2(d->cursorCaret.p2());


            QRectF rect = QRectF(caret1, caret2).normalized();
            if (!rect.isValid()) {
                if (rect.height() < 1) {
                    rect.adjust(0, -1, 0, 0);
                }
                if (rect.width() < 1) {
                    rect.adjust(0, 0, 1, 0);
                }

            }
            return rect.toAlignedRect();
        }
        break;
    case Qt::ImAnchorRectangle:
        // The platform integration will always define the anchor as the 'right side' handle.
        if (d->shape) {
            QPointF caret1(d->anchorCaret.p1());
            QPointF caret2(d->anchorCaret.p2());
            QRectF rect = QRectF(caret1, caret2).normalized();
            if (rect.isEmpty()) {
                if (rect.height() < 1) {
                    rect.adjust(0, -1, 0, 0);
                }
                if (rect.width() < 1) {
                    rect = rect.adjusted(-1, 0, 0, 0).normalized();
                }
            }
            return rect.toAlignedRect();
        }
        break;
    //case Qt::ImFont: // not sure what this is used for, but we cannot sent out without access to properties.
    case Qt::ImAbsolutePosition:
    case Qt::ImCursorPosition:
        if (d->shape) {
            return d->shape->indexForPos(d->pos);
        }
        break;
    case Qt::ImSurroundingText:
        if (d->shape) {
            QString surroundingText = d->shape->plainText();
            int preEditIndex = d->preEditCommand? d->shape->indexForPos(d->preEditStart): 0;
            surroundingText.remove(preEditIndex, d->preEditLength);
            return surroundingText;
        }
        break;
    case Qt::ImCurrentSelection:
        if (d->shape) {
            QString surroundingText = d->shape->plainText();
            int preEditIndex = d->preEditCommand? d->shape->indexForPos(d->preEditStart): 0;
            surroundingText.remove(preEditIndex, d->preEditLength);
            int start = d->shape->indexForPos(qMin(d->anchor, d->pos));
            int length = d->shape->indexForPos(qMax(d->anchor, d->pos)) - start;
            return surroundingText.mid(start, length);
        }
        break;
    case Qt::ImTextBeforeCursor:
        if (d->shape) {
            int start = d->shape->indexForPos(d->pos);
            QString surroundingText = d->shape->plainText();
            int preEditIndex = d->preEditCommand? d->shape->indexForPos(d->preEditStart): 0;
            surroundingText.remove(preEditIndex, d->preEditLength);
            return surroundingText.left(start);
        }
        break;
    case Qt::ImTextAfterCursor:
        if (d->shape) {
            int start = d->shape->indexForPos(d->pos);
            QString surroundingText = d->shape->plainText();
            int preEditIndex = d->preEditCommand? d->shape->indexForPos(d->preEditStart): 0;
            surroundingText.remove(preEditIndex, d->preEditLength);
            return surroundingText.right(start);
        }
        break;
    case Qt::ImMaximumTextLength:
        return QVariant(); // infinite text length!
        break;
    case Qt::ImAnchorPosition:
        if (d->shape) {
            return d->shape->indexForPos(d->anchor);
        }
        break;
    case Qt::ImHints:
        // It would be great to use Qt::ImhNoTextHandles or Qt::ImhNoEditMenu,
        // but neither are implemented for anything but web platform integration
        return Qt::ImhMultiLine;
        break;
    // case Qt::ImPreferredLanguage: // requires access to properties.
    // case Qt::ImPlatformData: // this is only for iOS at time of writing.
    case Qt::ImEnterKeyType:
        if (d->shape) {
            return Qt::EnterKeyDefault; // because input method hint is always multiline, this will show a return key.
        }
        break;
    // case Qt::ImInputItemClipRectangle // whether the input item is clipped?
    default:
        return QVariant();
    }
    return QVariant();
}

void SvgTextCursor::inputMethodEvent(QInputMethodEvent *event)
{
    dbgTools << "Commit:"<< event->commitString() << "predit:"<< event->preeditString();
    dbgTools << "Replacement:"<< event->replacementStart() << event->replacementLength();

    QRectF updateRect = d->shape? d->shape->boundingRect(): QRectF();
    d->blockQueryUpdates = true;
    SvgTextShapeManagerBlocker blocker(d->canvas->shapeManager());

    bool isGettingInput = !event->commitString().isEmpty() || !event->preeditString().isEmpty()
                || event->replacementLength() > 0;

    // Remove previous preedit string.
    if (d->preEditCommand) {
        d->preEditCommand->undo();
        d->preEditCommand = 0;
        d->preEditStart = -1;
        d->preEditLength = -1;
        updateRect |= d->shape? d->shape->boundingRect(): QRectF();
    }

    if (!d->shape || !isGettingInput) {
        blocker.unlock();
        d->canvas->shapeManager()->update(updateRect);
        event->ignore();
        return;
    }


    // remove the selection if any.
    addCommandToUndoAdapter(removeSelectionImpl(false));

    // set the text insertion pos to replacement start and also remove replacement length, if any.
    int originalPos = d->pos;
    int index = d->shape->indexForPos(d->pos) + event->replacementStart();
    d->pos = d->shape->posForIndex(index);
    if (event->replacementLength() > 0) {
        SvgTextRemoveCommand *cmd = new SvgTextRemoveCommand(d->shape,
                                                             index + event->replacementLength(),
                                                             originalPos,
                                                             d->anchor,
                                                             event->replacementLength(),
                                                             false);
        addCommandToUndoAdapter(cmd);
    }

    // add the commit string, if any.
    if (!event->commitString().isEmpty()) {
        insertText(event->commitString());
    }

    // set the selection...
    Q_FOREACH(const QInputMethodEvent::Attribute attribute, event->attributes()) {
        if (attribute.type == QInputMethodEvent::Selection) {
            d->pos = d->shape->posForIndex(attribute.start);
            int index = d->shape->indexForPos(d->pos);
            d->anchor = d->shape->posForIndex(index + attribute.length);
        }
    }


    // insert a preedit string, if any.
    if (!event->preeditString().isEmpty()) {
        int index = d->shape->indexForPos(d->pos);
        d->preEditCommand = new SvgTextInsertCommand(d->shape, d->pos, d->anchor, event->preeditString());
        d->preEditCommand->redo();
        d->preEditLength = event->preeditString().size();
        d->preEditStart = d->shape->posForIndex(index, true);
    } else {
        d->preEditCommand = 0;
    }

    // Apply the cursor offset for the preedit.
    QVector<IMEDecorationInfo> styleMap;
    Q_FOREACH(const QInputMethodEvent::Attribute attribute, event->attributes()) {
        dbgTools << "attribute: "<< attribute.type << "start: " << attribute.start
                 << "length: " << attribute.length << "val: " << attribute.value;
        // Text Format is about setting the look of the preedit string, and there can be multiple per event
        // we primarily interpret the underline. When a background color is set, we increase the underline
        // thickness, as that's what is actually supposed to happen according to the comments in the
        // platform input contexts for both macOS and Windows.

        if (attribute.type == QInputMethodEvent::TextFormat) {
            QVariant val = attribute.value;
            QTextCharFormat form = val.value<QTextFormat>().toCharFormat();

            if (attribute.length == 0 || attribute.start < 0 || !attribute.value.isValid()) {
                continue;
            }

            int positionA = -1;
            int positionB = -1;
            if (!styleMap.isEmpty()) {
                for (int i = 0; i < styleMap.size(); i++) {
                    if (attribute.start >= styleMap.at(i).start
                            && attribute.start < styleMap.at(i).start + styleMap.at(i).length) {
                        positionA = i;
                    }
                    if (attribute.start + attribute.length > styleMap.at(i).start
                            && attribute.start + attribute.length <= styleMap.at(i).start + styleMap.at(i).length) {
                        positionB = i;
                    }
                }

                if (positionA > -1 && positionA == positionB) {
                    IMEDecorationInfo decoration1 = styleMap.at(positionA);
                    IMEDecorationInfo decoration2 = decoration1;
                    IMEDecorationInfo decoration3 = decoration1;
                    decoration3.start = (attribute.start+attribute.length);
                    decoration3.length = (decoration1.start + decoration1.length) - decoration3.start;
                    decoration1.length = attribute.start - decoration1.start;
                    decoration2.start = attribute.start;
                    decoration2.length = attribute.length;
                    if (decoration1.length > 0) {
                        styleMap[positionA] = decoration1;
                        if (decoration2.length > 0) {
                            positionA += 1;
                            styleMap.insert(positionA, decoration2);
                        }
                    } else {
                        styleMap[positionA] = decoration2;
                    }
                    if (decoration3.length > 0) {
                        styleMap.insert(positionA + 1, decoration3);
                    }
                } else if (positionA > -1 && positionB > -1
                           && positionA != positionB) {
                    IMEDecorationInfo decoration1 = styleMap.at(positionA);
                    IMEDecorationInfo decoration2 = decoration1;
                    IMEDecorationInfo decoration3 = styleMap.at(positionB);
                    IMEDecorationInfo decoration4 = decoration3;
                    decoration2.length = (decoration1.start + decoration1.length) - attribute.start;
                    decoration1.length =  attribute.start - decoration1.start;
                    decoration2.start  =  attribute.start;

                    decoration4.start = (attribute.start+attribute.length);
                    decoration3.length = (decoration3.start + decoration3.length) - decoration4.start;
                    decoration3.length = decoration4.start - decoration3.start;
                    if (decoration1.length > 0) {
                        styleMap[positionA] = decoration1;
                        if (decoration2.length > 0) {
                            positionA += 1;
                            styleMap.insert(positionA, decoration2);
                        }
                    } else {
                        styleMap[positionA] = decoration2;
                    }

                    if (decoration3.length > 0) {
                        styleMap[positionB] = decoration3;
                        if (decoration4.length > 0) {
                            styleMap.insert(positionB + 1, decoration4);
                        }
                    } else {
                        styleMap[positionB] = decoration4;
                    }
                }
            }

            if (positionA > -1 && !styleMap.isEmpty()) {

                for(int i = positionA; i <= positionB; i++) {
                    IMEDecorationInfo decoration = styleMap.at(i);
                    decoration.setDecorationFromQTextCharFormat(form);
                    styleMap[i] = decoration;
                }

            } else {
                IMEDecorationInfo decoration;
                decoration.start = attribute.start;
                decoration.length = attribute.length;
                decoration.setDecorationFromQTextCharFormat(form);
                styleMap.append(decoration);
            }

        // QInputMethodEvent::Language is about setting the locale on the given  preedit string, which is not possible yet.
        // QInputMethodEvent::Ruby is supposedly ruby info for the preedit string, but none of the platform integrations
        // actually implement this at time of writing, and it may have been something from a previous live of Qt's.
        } else if (attribute.type == QInputMethodEvent::Cursor) {
            if (d->preEditStart < 0) {
                d->anchor = d->pos;
            } else {
                int index = d->shape->indexForPos(d->preEditStart);
                d->pos = d->shape->posForIndex(index + attribute.start);
                d->anchor = d->pos;
            }

            // attribute value is the cursor color, and should be used to paint the cursor.
            // attribute length is about whether the cursor should be visible at all...
        }
    }

    blocker.unlock();
    d->blockQueryUpdates = false;
    qApp->inputMethod()->update(Qt::ImQueryInput);
    updateRect |= d->shape->boundingRect();
    // TODO: replace with KoShapeBulkActionLock
    d->shape->updateAbsolute(updateRect);
    d->styleMap = styleMap;
    updateIMEDecoration();
    updateSelection();
    updateCursor();
    event->accept();
}

void SvgTextCursor::blinkCursor()
{
    if (d->shape) {
        Q_EMIT updateCursorDecoration(d->shape->shapeToDocument(d->cursorShape.boundingRect()) | d->oldCursorRect);
        d->cursorVisible = !d->cursorVisible;
    }
}

void SvgTextCursor::stopBlinkCursor()
{
    d->cursorFlash.stop();
    d->cursorFlashLimit.stop();
    d->cursorVisible = true;
    if (d->shape) {
        Q_EMIT updateCursorDecoration(d->shape->shapeToDocument(d->cursorShape.boundingRect()) | d->oldCursorRect);
    }
}

void SvgTextCursor::updateInputMethodItemTransform()
{
    // Mockcanvas in the tests has no window.
    if (!d->canvas->canvasWidget()) {
        return;
    }
    QPoint pos = d->canvas->canvasWidget()->mapTo(d->canvas->canvasWidget()->window(), QPoint());
    QTransform widgetToWindow = QTransform::fromTranslate(pos.x(), pos.y());
    QTransform inputItemTransform = widgetToWindow;
    QRectF inputRect = d->canvas->canvasWidget()->geometry();
    if (d->shape) {
        inputRect = d->shape->outlineRect().normalized();
        QTransform shapeTransform = d->shape->absoluteTransformation();
        QTransform docToView = d->canvas->viewConverter()->documentToView();
        QTransform viewToWidget = d->canvas->viewConverter()->viewToWidget();
        inputItemTransform = shapeTransform * docToView * viewToWidget * widgetToWindow;
    }
    qApp->inputMethod()->setInputItemTransform(inputItemTransform);
    qApp->inputMethod()->setInputItemRectangle(inputRect);
    if (!d->blockQueryUpdates) {
        qApp->inputMethod()->update(Qt::ImQueryInput);
    }
}

void SvgTextCursor::canvasResourceChanged(int key, const QVariant &value)
{
    if (!d->shape || (key != KoCanvasResource::ForegroundColor && key != KoCanvasResource::BackgroundColor))
        return;

    KoSvgTextProperties props;
    KoSvgTextProperties shapeProps = hasSelection()? d->shape->propertiesForPos(d->pos, true): d->shape->textProperties();
    if (key == KoCanvasResource::ForegroundColor) {
        QSharedPointer<KoShapeBackground> bg(new KoColorBackground(value.value<KoColor>().toQColor()));
        if (bg != shapeProps.background()) {
            props.setProperty(KoSvgTextProperties::FillId,
                              QVariant::fromValue(KoSvgText::BackgroundProperty(bg)));
        }
    } else if (key == KoCanvasResource::BackgroundColor) {
        QSharedPointer<KoShapeStroke> stroke(new KoShapeStroke());
        if (shapeProps.stroke()) {
            stroke = qSharedPointerDynamicCast<KoShapeStroke>(shapeProps.stroke());
        }
        stroke->setColor(value.value<KoColor>().toQColor());
        if (stroke != shapeProps.stroke()) {
            props.setProperty(KoSvgTextProperties::StrokeId,
                              QVariant::fromValue(KoSvgText::StrokeProperty(stroke)));
        }
    }
    if (!props.isEmpty()) {
        mergePropertiesIntoSelection(props, QSet<KoSvgTextProperties::PropertyId>(), !hasSelection());
    }
}

void SvgTextCursor::toggleProperty(KoSvgTextProperties::PropertyId property)
{
    if (d->shape) {
        QVariant newVal;
        QList<KoSvgTextProperties> p = d->shape->propertiesForRange(qMin(d->pos, d->anchor), qMax(d->pos, d->anchor));
        for(auto it = p.begin(); it != p.end(); it++) {
            if (property == KoSvgTextProperties::FontWeightId) {
                int value = it->property(property, QVariant(400)).toInt();
                newVal = value == 400? QVariant(700): QVariant(400);
                if (value == 400) break;
            } else if (property == KoSvgTextProperties::FontStyleId) {
                KoSvgText::CssFontStyleData value = it->property(property, QVariant(QFont::StyleNormal)).value<KoSvgText::CssFontStyleData>();
                KoSvgText::CssFontStyleData newSlant = value;
                if (value.style == QFont::StyleNormal) {
                    newSlant.style = QFont::StyleItalic;
                } else {
                    newSlant.style = QFont::StyleNormal;
                    newSlant.slantValue.customValue = 0;
                    newSlant.slantValue.isAuto = true;
                }
                newVal = QVariant::fromValue(newSlant);
                if (value.style == QFont::StyleNormal) break;
            } else if (property == KoSvgTextProperties::TextDecorationLineId) {
                KoSvgText::TextDecorations decor = it->propertyOrDefault(KoSvgTextProperties::TextDecorationLineId).value<KoSvgText::TextDecorations>();
                KoSvgText::TextDecorations newDecor;
                if (decor.testFlag(KoSvgText::DecorationUnderline)) {
                    newDecor.setFlag(KoSvgText::DecorationUnderline, false);
                    newVal = QVariant::fromValue(newDecor);
                } else {
                    newDecor.setFlag(KoSvgText::DecorationUnderline, true);
                    newVal = QVariant::fromValue(newDecor);
                    break;
                }
            }
        }
        KoSvgTextProperties properties;
        properties.setProperty(property, newVal);
        mergePropertiesIntoSelection(properties);
    }
}

void SvgTextCursor::propertyAction()
{
    QAction *action = dynamic_cast<QAction*>(QObject::sender());
    if (!action || !d->shape) return;

    QList<KoSvgTextProperties> p = d->shape->propertiesForRange(qMin(d->pos, d->anchor), qMax(d->pos, d->anchor));
    KoSvgTextProperties properties = SvgTextShortCuts::getModifiedProperties(action, p);
    if (properties.isEmpty()) return;
    mergePropertiesIntoSelection(properties);
}

void SvgTextCursor::clearFormattingAction()
{
    KoSvgTextProperties props;

    QSet<KoSvgTextProperties::PropertyId> ids;

    for (int i = 0; i < int(KoSvgTextProperties::LastPropertyId); i++) {
        ids.insert(KoSvgTextProperties::PropertyId(i));
    }

    mergePropertiesIntoSelection(props, ids);
}

bool SvgTextCursor::hasSelection()
{
    return d->pos != d->anchor;
}

void SvgTextCursor::notifyShapeChanged(KoShape::ChangeType type, KoShape *shape)
{
    Q_UNUSED(type);
    Q_UNUSED(shape);
    d->pos = d->shape->posForIndex(d->posIndex);
    d->anchor = d->shape->posForIndex(d->anchorIndex);
    updateCursor(true);
    updateSelection();
    updateInputMethodItemTransform();
    updateTypeSettingDecoration();
}

void SvgTextCursor::notifyCursorPosChanged(int pos, int anchor)
{
    d->pos = pos;
    d->anchor = anchor;
    updateCursor();
    updateSelection();
    updateTypeSettingDecoration();
}

void SvgTextCursor::notifyMarkupChanged()
{
    d->interface->emitSelectionChange();
    d->interface->emitCharacterSelectionChange();
    updateTypeSettingDecoration();
}

void SvgTextCursor::keyPressEvent(QKeyEvent *event)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(d->shape);

    updateModifiers(event->modifiers());

    if (d->preEditCommand) {
        //MacOS will keep sending keyboard events during IME handling.
        event->accept();
        return;
    }

    bool select = event->modifiers().testFlag(Qt::ShiftModifier);

    if (!((Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier) & event->modifiers())) {

        switch (event->key()) {
        case Qt::Key_Right:
            moveCursor(SvgTextCursor::MoveRight, !select);
            event->accept();
            break;
        case Qt::Key_Left:
            moveCursor(SvgTextCursor::MoveLeft, !select);
            event->accept();
            break;
        case Qt::Key_Up:
            moveCursor(SvgTextCursor::MoveUp, !select);
            event->accept();
            break;
        case Qt::Key_Down:
            moveCursor(SvgTextCursor::MoveDown, !select);
            event->accept();
            break;
        case Qt::Key_Delete:
            removeText(MoveNone, MoveNextChar);
            event->accept();
            break;
        case Qt::Key_Backspace:
            removeLastCodePoint();
            event->accept();
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            insertText("\n");
            event->accept();
            break;
        default:
            event->ignore();
        }

        if (event->isAccepted()) {
            return;
        }
    }
    if (acceptableInput(event)) {
        insertText(event->text());
        event->accept();
        return;
    }

    KoSvgTextProperties props = d->shape->textProperties();

    KoSvgText::WritingMode mode = KoSvgText::WritingMode(props.propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
    KoSvgText::Direction direction = KoSvgText::Direction(props.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());

    // Qt's keysequence stuff doesn't handle vertical, so to test all the standard keyboard shortcuts as if it did,
    // we reinterpret the direction keys according to direction and writing mode, and test against that.

    int newKey = event->key();

    if (direction == KoSvgText::DirectionRightToLeft) {
        switch (newKey) {
        case Qt::Key_Left:
            newKey = Qt::Key_Right;
            break;
        case Qt::Key_Right:
            newKey = Qt::Key_Left;
            break;
        default:
            break;
        }
    }

    if (mode == KoSvgText::VerticalRL) {
        switch (newKey) {
        case Qt::Key_Left:
            newKey = Qt::Key_Down;
            break;
        case Qt::Key_Right:
            newKey = Qt::Key_Up;
            break;
        case Qt::Key_Up:
            newKey = Qt::Key_Left;
            break;
        case Qt::Key_Down:
            newKey = Qt::Key_Right;
            break;
        default:
            break;
        }
    } else if (mode == KoSvgText::VerticalRL) {
        switch (newKey) {
        case Qt::Key_Left:
            newKey = Qt::Key_Up;
            break;
        case Qt::Key_Right:
            newKey = Qt::Key_Down;
            break;
        case Qt::Key_Up:
            newKey = Qt::Key_Left;
            break;
        case Qt::Key_Down:
            newKey = Qt::Key_Right;
            break;
        default:
            break;
        }
    }

    QKeySequence testSequence(event->modifiers() | newKey);


    // Note for future, when we have format changing actions:
    // We'll need to test format change actions before the standard
    // keys, as one of the standard keys for deleting a line is ctrl+u
    // which would probably be expected to do underline before deleting.

    Q_FOREACH(QAction *action, d->actions) {
        if (action->shortcut() == testSequence) {
            event->accept();
            action->trigger();
            return;
        }
    }

    // This first set is already tested above, however, if they still
    // match, then it's one of the extra sequences for MacOs, which
    // seem to be purely logical, instead of the visual set we tested
    // above.
    if (testSequence == QKeySequence::MoveToNextChar) {
        moveCursor(SvgTextCursor::MoveNextChar, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectNextChar) {
        moveCursor(SvgTextCursor::MoveNextChar, false);
        event->accept();
    } else if (testSequence == QKeySequence::MoveToPreviousChar) {
        moveCursor(SvgTextCursor::MovePreviousChar, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectPreviousChar) {
        moveCursor(SvgTextCursor::MovePreviousChar, false);
        event->accept();
    } else if (testSequence == QKeySequence::MoveToNextLine) {
        moveCursor(SvgTextCursor::MoveNextLine, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectNextLine) {
        moveCursor(SvgTextCursor::MoveNextLine, false);
        event->accept();
    } else if (testSequence == QKeySequence::MoveToPreviousLine) {
        moveCursor(SvgTextCursor::MovePreviousLine, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectPreviousLine) {
        moveCursor(SvgTextCursor::MovePreviousLine, false);
        event->accept();

    } else if (testSequence == QKeySequence::MoveToNextWord) {
        moveCursor(SvgTextCursor::MoveWordEnd, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectNextWord) {
        moveCursor(SvgTextCursor::MoveWordEnd, false);
        event->accept();
    } else if (testSequence == QKeySequence::MoveToPreviousWord) {
        moveCursor(SvgTextCursor::MoveWordStart, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectPreviousWord) {
        moveCursor(SvgTextCursor::MoveWordStart, false);
        event->accept();

    } else if (testSequence == QKeySequence::MoveToStartOfLine) {
        moveCursor(SvgTextCursor::MoveLineStart, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectStartOfLine) {
        moveCursor(SvgTextCursor::MoveLineStart, false);
        event->accept();
    } else if (testSequence == QKeySequence::MoveToEndOfLine) {
        moveCursor(SvgTextCursor::MoveLineEnd, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectEndOfLine) {
        moveCursor(SvgTextCursor::MoveLineEnd, false);
        event->accept();

    } else if (testSequence == QKeySequence::MoveToStartOfBlock
               || testSequence == QKeySequence::MoveToStartOfDocument) {
        moveCursor(SvgTextCursor::ParagraphStart, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectStartOfBlock
               || testSequence == QKeySequence::SelectStartOfDocument) {
        moveCursor(SvgTextCursor::ParagraphStart, false);
        event->accept();

    } else if (testSequence == QKeySequence::MoveToEndOfBlock
               || testSequence == QKeySequence::MoveToEndOfDocument) {
        moveCursor(SvgTextCursor::ParagraphEnd, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectEndOfBlock
               || testSequence == QKeySequence::SelectEndOfDocument) {
        moveCursor(SvgTextCursor::ParagraphEnd, false);
        event->accept();

    }else if (testSequence == QKeySequence::DeleteStartOfWord) {
        removeText(MoveWordStart, MoveNone);
        event->accept();
    } else if (testSequence == QKeySequence::DeleteEndOfWord) {
        removeText(MoveNone, MoveWordEnd);
        event->accept();
    } else if (testSequence == QKeySequence::DeleteEndOfLine) {
        removeText(MoveNone, MoveLineEnd);
        event->accept();
    } else if (testSequence == QKeySequence::DeleteCompleteLine) {
        removeText(MoveLineStart, MoveLineEnd);
        event->accept();
    } else if (testSequence == QKeySequence::Backspace) {
        removeLastCodePoint();
        event->accept();
    } else if (testSequence == QKeySequence::Delete) {
        removeText(MoveNone, MoveNextChar);
        event->accept();

    } else if (testSequence == QKeySequence::InsertLineSeparator
               || testSequence == QKeySequence::InsertParagraphSeparator) {
        insertText("\n");
        event->accept();
    } else {
        event->ignore();
    }
}

void SvgTextCursor::updateModifiers(const Qt::KeyboardModifiers modifiers)
{
    d->lastKnownModifiers = modifiers;
    updateTypeSettingDecoration();
}

bool SvgTextCursor::isAddingCommand() const
{
    return d->isAddingCommand;
}

void SvgTextCursor::focusIn()
{
    d->cursorFlash.start();
    d->cursorFlashLimit.start();
    d->cursorVisible = false;
    blinkCursor();
}

void SvgTextCursor::focusOut()
{
    stopBlinkCursor();
}

bool SvgTextCursor::registerPropertyAction(QAction *action, const QString &name)
{
    if (SvgTextShortCuts::configureAction(action, name)) {
        d->actions.append(action);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(propertyAction()));
        return true;
    } else if (name == "svg_insert_special_character") {
        d->actions.append(action);
        connect(action, SIGNAL(triggered(bool)), this, SIGNAL(sigOpenGlyphPalette()));
        return true;
    } else if (name == "svg_paste_rich_text") {
        d->actions.append(action);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(pasteRichText()));
        return true;
    } else if (name == "svg_paste_plain_text") {
        d->actions.append(action);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(pastePlainText()));
        return true;
    } else if (name == "svg_remove_transforms_from_range") {
        d->actions.append(action);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(removeTransformsFromRange()));
        return true;
    } else if (name == "svg_clear_formatting") {
        d->actions.append(action);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(clearFormattingAction()));
        return true;
    } else if (action) {
        d->actions.append(action);
        return true;
    }
    return false;
}

KoSvgTextPropertiesInterface *SvgTextCursor::textPropertyInterface()
{
    return d->interface;
}

void SvgTextCursor::updateCursor(bool firstUpdate)
{
    if (d->shape) {
        d->oldCursorRect = d->shape->shapeToDocument(d->cursorShape.boundingRect());
        d->posIndex = d->shape->indexForPos(d->pos);
        d->anchorIndex = d->shape->indexForPos(d->anchor);
        emit selectionChanged();
        updateTypeSettingDecoration();
    }
    d->cursorColor = QColor();
    d->cursorShape = d->shape? d->shape->cursorForPos(d->pos, d->cursorCaret, d->cursorColor): QPainterPath();

    if (!d->blockQueryUpdates) {
        qApp->inputMethod()->update(Qt::ImQueryInput);
    }
    updateCanvasResources();
    d->interface->emitCharacterSelectionChange();
    if (!(d->canvas->canvasWidget() && d->canvas->canvasController())) {
        // Mockcanvas in the tests has neither.
        return;
    }
    if (d->shape && !firstUpdate) {
        QRectF rect = d->shape->shapeToDocument(d->cursorShape.boundingRect());
        d->canvas->canvasController()->ensureVisibleDoc(rect, false);
    }
    if (d->canvas->canvasWidget()->hasFocus()) {
        d->cursorFlash.start();
        d->cursorFlashLimit.start();
        d->cursorVisible = false;
        blinkCursor();
    }
}

void SvgTextCursor::updateSelection()
{
    if (d->shape) {
        d->oldSelectionRect = d->shape->shapeToDocument(d->selection.boundingRect());
        d->shape->cursorForPos(d->anchor, d->anchorCaret, d->cursorColor);
        d->selection = d->shape->selectionBoxes(d->pos, d->anchor);
        Q_EMIT updateCursorDecoration(d->shape->shapeToDocument(d->selection.boundingRect()) | d->oldSelectionRect);
    }
}

void SvgTextCursor::updateIMEDecoration()
{
    if (d->shape) {
        d->oldIMEDecorationRect = d->shape->shapeToDocument(d->IMEDecoration.boundingRect());
        KoSvgText::TextDecorations decor;
        decor.setFlag(KoSvgText::DecorationUnderline, true);
        d->IMEDecoration = QPainterPath();
        if (d->preEditCommand) {
            Q_FOREACH(const IMEDecorationInfo info,  d->styleMap) {

                int startIndex = d->shape->indexForPos(d->preEditStart) + info.start;
                int endIndex = startIndex + info.length;
                qreal minimum = d->canvas->viewToDocument(QPointF(1, 1)).x();
                d->IMEDecoration.addPath(d->shape->underlines(d->shape->posForIndex(startIndex),
                                                              d->shape->posForIndex(endIndex),
                                                              info.decor,
                                                              info.style,
                                                              minimum,
                                                              info.thick));
                d->IMEDecoration.setFillRule(Qt::WindingFill);
            }
        }

        Q_EMIT updateCursorDecoration(d->shape->shapeToDocument(d->IMEDecoration.boundingRect()) | d->oldIMEDecorationRect);
    }
}

void SvgTextCursor::updateTypeSettingDecoration()
{
    QRectF updateRect;
    if (d->shape && d->typeSettingMode) {

        QList<KoSvgTextCharacterInfo> infos =
                d->shape->getPositionsAndRotationsForRange(d->pos, d->anchor);
        if (infos.size() < 1) return;

        const bool rtl = infos.first().rtl;

        KoSvgTextCharacterInfo first = infos.first();
        KoSvgTextCharacterInfo last = infos.last();
        if (infos.size() > 1) {
            std::sort(infos.begin(), infos.end(), KoSvgTextCharacterInfo::visualLessThan);
            for (auto it = infos.begin(); it != infos.end(); it++) {
                if (it->visualIndex >= 0) {
                    first = *it;
                    break;
                }
            }
            for (auto it = infos.rbegin(); it != infos.rend(); it++) {
                if (it->visualIndex >= 0) {
                    last = *it;
                    break;
                }
            }
        }

        QTransform t = QTransform::fromTranslate(last.finalPos.x(), last.finalPos.y());
        t.rotate(last.rotateDeg);
        last.finalPos = t.map(last.advance);

        //
        d->typeSettingDecor.handles.first = rtl? last.finalPos: first.finalPos;
        d->typeSettingDecor.handles.second = rtl? first.finalPos: last.finalPos;

        // This could be better, it's quite clunky right now.
        // It'd be better if we could get KoSvgTextNodeIndex for a pos, but that one is only implemented in shape branch...
        d->typeSettingDecor.paths = QMap<SvgTextCursor::TypeSettingModeHandle, QPainterPath>();
        d->typeSettingDecor.baselines = QMap<SvgTextCursor::TypeSettingModeHandle, QPainterPath>();
        QList<KoSvgTextCharacterInfo> metricInfos = infos;
        if (d->pos == d->anchor) {
            metricInfos = d->shape->getPositionsAndRotationsForRange(0, d->shape->posForIndex(d->shape->plainText().size()));
        }
        for (auto it = metricInfos.begin(); it != metricInfos.end(); it++) {
            const int currentPos = (d->pos == d->anchor)? -1 :d->shape->posForIndex(it->logicalIndex);
            const KoSvgTextProperties props = d->shape->propertiesForPos(currentPos, true);
            KoSvgText::LineHeightInfo lineHeight = props.propertyOrDefault(KoSvgTextProperties::LineHeightId).value<KoSvgText::LineHeightInfo>();

            KoSvgText::FontMetrics metrics = (d->pos == d->anchor)? props.metrics(true, true) :it->metrics;
            const bool isHorizontal = d->shape->writingMode() == KoSvgText::HorizontalTB;

            const qreal scaleMetrics = props.fontSize().value/qreal(metrics.fontSize);
            const int lineGap = lineHeight.isNormal? metrics.lineGap: (lineHeight.length.value/scaleMetrics)-(metrics.ascender-metrics.descender);

            QTransform t = QTransform::fromTranslate(it->finalPos.x(), it->finalPos.y());
            t.rotate(it->rotateDeg);

            const QMap<SvgTextCursor::TypeSettingModeHandle, int> types
                    = typeSettingBaselinesFromMetrics(metrics, lineGap, isHorizontal);

            Q_FOREACH(SvgTextCursor::TypeSettingModeHandle handle, types.keys()) {
                const int metric = types.value(handle);
                QPointF offset = isHorizontal? QPointF(0, -(metric*scaleMetrics)): QPointF(metric*scaleMetrics, 0);
                QPainterPath p = d->typeSettingDecor.baselines.value(handle);
                p.moveTo(t.map(offset));
                p.lineTo(t.map(offset+it->advance));
                d->typeSettingDecor.baselines.insert(handle, p);
            }
        }
        const QList<SvgTextCursor::TypeSettingModeHandle> nonBaselines = {
            LineHeightTop, Ascender, BaselineShift, Descender, LineHeightBottom
        };
        Q_FOREACH(SvgTextCursor::TypeSettingModeHandle handle, nonBaselines) {
            d->typeSettingDecor.paths.insert(handle, d->typeSettingDecor.baselines.value(handle));
        }
        d->typeSettingDecor.baselines.remove(LineHeightTop);
        d->typeSettingDecor.baselines.remove(LineHeightBottom);


        updateRect = d->shape->shapeToDocument(d->typeSettingDecor.boundingRect(d->handleRadius));
    }
    Q_EMIT updateCursorDecoration(updateRect | d->oldTypeSettingRect);
    d->oldTypeSettingRect = updateRect;
}

void SvgTextCursor::addCommandToUndoAdapter(KUndo2Command *cmd)
{
    if (d->canvas) {
        if (cmd) {
            d->isAddingCommand = true;
            d->canvas->addCommand(cmd);
            d->isAddingCommand = false;
        }
    }
}

int SvgTextCursor::moveModeResult(const SvgTextCursor::MoveMode mode, int &pos, bool visual) const
{
    int newPos = pos;
    switch (mode) {
        case MoveNone:
        break;
    case MoveLeft:
        newPos = d->shape->posLeft(pos, visual);
        break;
    case MoveRight:
        newPos = d->shape->posRight(pos, visual);
        break;
    case MoveUp:
        newPos = d->shape->posUp(pos, visual);
        break;
    case MoveDown:
        newPos = d->shape->posDown(pos, visual);
        break;
    case MovePreviousChar:
        newPos = d->shape->previousIndex(pos);
        break;
    case MoveNextChar:
        newPos = d->shape->nextIndex(pos);
        break;
    case MovePreviousLine:
        newPos = d->shape->previousLine(pos);
        break;
    case MoveNextLine:
        newPos = d->shape->nextLine(pos);
        break;
    case MoveWordLeft:
        newPos = d->shape->wordLeft(pos, visual);
        if (newPos == pos) {
            newPos = d->shape->posLeft(pos, visual);
            newPos = d->shape->wordLeft(newPos, visual);
        }
        break;
    case MoveWordRight:
        newPos = d->shape->wordRight(pos, visual);
        if (newPos == pos) {
            newPos = d->shape->posRight(pos, visual);
            newPos = d->shape->wordRight(newPos, visual);
        }
        break;
    case MoveWordStart:
        newPos = d->shape->wordStart(pos);
        if (newPos == pos) {
            newPos = d->shape->previousIndex(pos);
            newPos = d->shape->wordStart(newPos);
        }
        break;
    case MoveWordEnd:
        newPos = d->shape->wordEnd(pos);
        if (newPos == pos) {
            newPos = d->shape->nextIndex(pos);
            newPos = d->shape->wordEnd(newPos);
        }
        break;
    case MoveLineStart:
        newPos = d->shape->lineStart(pos);
        break;
    case MoveLineEnd:
        newPos = d->shape->lineEnd(pos);
        break;
    case ParagraphStart:
        newPos = 0;
        break;
    case ParagraphEnd:
        newPos = d->shape->posForIndex(d->shape->plainText().size());
        break;
    }
    return newPos;
}

/// More or less copied from bool QInputControl::isAcceptableInput(const QKeyEvent *event) const
bool SvgTextCursor::acceptableInput(const QKeyEvent *event) const
{
    const QString text = event->text();
    if (text.isEmpty())
        return false;
    const QChar c = text.at(0);
    // Formatting characters such as ZWNJ, ZWJ, RLM, etc. This needs to go before the
    // next test, since CTRL+SHIFT is sometimes used to input it on Windows.
    if (c.category() == QChar::Other_Format)
        return true;
    // QTBUG-35734: ignore Ctrl/Ctrl+Shift; accept only AltGr (Alt+Ctrl) on German keyboards
    if (event->modifiers() == Qt::ControlModifier
            || event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
        return false;
    }
    if (c.isPrint())
        return true;
    if (c.category() == QChar::Other_PrivateUse)
        return true;
    if (c == QLatin1Char('\t'))
        return true;
    return false;
}

void SvgTextCursor::commitIMEPreEdit()
{
    if (!d->preEditCommand) {
        return;
    }

    qApp->inputMethod()->commit();

    if (!d->preEditCommand) {
        return;
    }

    d->preEditCommand->undo();
    d->preEditCommand = nullptr;
    d->preEditStart = -1;
    d->preEditLength = 0;
    updateIMEDecoration();
    updateCursor();
}

void SvgTextCursor::updateCanvasResources()
{
    // Only update canvas resources when there's no selection.
    // This relies on Krita not setting anything on the text when there's no selection.
    if (d->shape && d->canvas->resourceManager() && d->pos == d->anchor) {
        KoSvgTextProperties props = hasSelection()? d->shape->propertiesForPos(d->pos, true): d->shape->textProperties();
        KoColorBackground *bg = dynamic_cast<KoColorBackground *>(props.background().data());
        if (bg) {
            KoColor c;
            c.fromQColor(bg->color());
            c.setOpacity(1.0);
            if (c != d->canvas->resourceManager()->foregroundColor()) {
                d->canvas->resourceManager()->setForegroundColor(c);
            }
        }
        KoShapeStroke *stroke = dynamic_cast<KoShapeStroke *>(props.stroke().data());
        if (stroke && stroke->color().isValid()) {
            KoColor c;
            c.fromQColor(stroke->color());
            c.setOpacity(1.0);
            if (c != d->canvas->resourceManager()->backgroundColor()) {
                d->canvas->resourceManager()->setBackgroundColor(c);
            }
        }
        Q_FOREACH (QAction *action, d->actions) {
            if (action->isCheckable()) {
                action->setChecked(SvgTextShortCuts::actionEnabled(action, {props}));
            }
        }
    }
}

struct SvgTextCursorPropertyInterface::Private {
    Private(SvgTextCursor *parent)
        : parent(parent)
        , compressor(10, KisSignalCompressor::POSTPONE)
        , characterCompressor(10, KisSignalCompressor::POSTPONE){}
    SvgTextCursor *parent{nullptr};
    KisSignalCompressor compressor;
    KisSignalCompressor characterCompressor;
};

SvgTextCursorPropertyInterface::SvgTextCursorPropertyInterface(SvgTextCursor *parent)
    : KoSvgTextPropertiesInterface(parent), d(new Private(parent))
{
    connect(&d->compressor, SIGNAL(timeout()), this, SIGNAL(textSelectionChanged()));
    connect(&d->characterCompressor, SIGNAL(timeout()), this, SIGNAL(textCharacterSelectionChanged()));
}

SvgTextCursorPropertyInterface::~SvgTextCursorPropertyInterface()
{

}
QList<KoSvgTextProperties> SvgTextCursorPropertyInterface::getSelectedProperties()
{
    return d->parent->propertiesForShape();
}

QList<KoSvgTextProperties> SvgTextCursorPropertyInterface::getCharacterProperties()
{
    return d->parent->propertiesForRange();
}

KoSvgTextProperties SvgTextCursorPropertyInterface::getInheritedProperties()
{
    // 9 times out of 10 this is correct, though we could do better by actually
    // getting inherited properties for the range and not just defaulting to the paragraph.
    return (d->parent->shape())? d->parent->shape()->textProperties(): KoSvgTextProperties();
}

void SvgTextCursorPropertyInterface::setPropertiesOnSelected(KoSvgTextProperties properties, QSet<KoSvgTextProperties::PropertyId> removeProperties)
{
    d->parent->mergePropertiesIntoSelection(properties, removeProperties, true);
}

void SvgTextCursorPropertyInterface::setCharacterPropertiesOnSelected(KoSvgTextProperties properties, QSet<KoSvgTextProperties::PropertyId> removeProperties)
{
    d->parent->mergePropertiesIntoSelection(properties, removeProperties, false, true);
}

bool SvgTextCursorPropertyInterface::spanSelection()
{
    return d->parent->hasSelection();
}

bool SvgTextCursorPropertyInterface::characterPropertiesEnabled()
{
    return true;
}

void SvgTextCursorPropertyInterface::emitSelectionChange()
{
    // Don't bother updating the selection when there's no shape
    // this is so we can use the text properties last used to create new texts.
    if (!d->parent->shape()) return;
    d->compressor.start();
}

void SvgTextCursorPropertyInterface::emitCharacterSelectionChange()
{
    d->characterCompressor.start();
}
