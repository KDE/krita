/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SvgTextTool.h"
#include "KoSvgTextProperties.h"
#include "KoSvgTextShape.h"
#include "KoSvgTextShapeMarkupConverter.h"
#include "SvgCreateTextStrategy.h"
#include "SvgInlineSizeChangeCommand.h"
#include "SvgInlineSizeChangeStrategy.h"
#include "SvgSelectTextStrategy.h"
#include "SvgInlineSizeHelper.h"
#include "SvgMoveTextCommand.h"
#include "SvgMoveTextStrategy.h"
#include "SvgTextChangeCommand.h"
#include "SvgTextEditor.h"
#include "SvgTextRemoveCommand.h"
#include "SvgConvertTextTypeCommand.h"
#include "SvgTextShortCuts.h"
#include "SvgTextToolOptionsModel.h"

#include <QPainterPath>
#include <QDesktopServices>
#include <QApplication>
#include <QStyle>
#include <QDockWidget>
#include <QQuickItem>

#include <klocalizedstring.h>

#include <KisPart.h>
#include <kis_canvas2.h>
#include <KSharedConfig>
#include "kis_assert.h"
#include <kis_coordinates_converter.h>

#include <KoFileDialog.h>
#include <KoIcon.h>
#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoShapeController.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactoryBase.h>
#include <KoPointerEvent.h>
#include <KoProperties.h>
#include <KoSelectedShapesProxy.h>
#include "KoToolManager.h"
#include <KoShapeFillWrapper.h>
#include "KoCanvasResourceProvider.h"
#include <KoPathShape.h>
#include <KoPathSegment.h>

#include <KisResourceModelProvider.h>
#include <KoCssStylePreset.h>
#include <KoSvgTextPropertyData.h>
#include <KoColorBackground.h>
#include <KisResourceModel.h>

#include <KisTextPropertiesManager.h>
#include <KisViewManager.h>
#include <KisQQuickWidget.h>
#include <QQmlError>
#include <KisMainWindow.h>

#include "KisHandlePainterHelper.h"
#include "kis_tool_utils.h"
#include "kis_debug.h"
#include <commands/KoKeepShapesSelectedCommand.h>


using SvgInlineSizeHelper::InlineSizeInfo;

constexpr double INLINE_SIZE_DASHES_PATTERN_A = 4.0; /// Size of the visible part of the inline-size handle dashes.
constexpr double INLINE_SIZE_DASHES_PATTERN_B = 8.0; /// Size of the hidden part of the inline-size handle dashes.
constexpr int INLINE_SIZE_DASHES_PATTERN_LENGTH = 3; /// Total amount of trailing dashes on inline-size handles.
constexpr double INLINE_SIZE_HANDLE_THICKNESS = 1.0; /// Linethickness.


static bool debugEnabled()
{
    static const bool debugEnabled = !qEnvironmentVariableIsEmpty("KRITA_DEBUG_TEXTTOOL");
    return debugEnabled;
}

SvgTextTool::SvgTextTool(KoCanvasBase *canvas)
    : KoToolBase(canvas)
    , m_textCursor(canvas)
    , m_optionManager(new SvgTextToolOptionsManager(this))
{
     // TODO: figure out whether we should use system config for this, Windows and GTK have values for it, but Qt and MacOS don't(?).
    int cursorFlashLimit = 5000;
    m_textCursor.setCaretSetting(QApplication::style()->pixelMetric(QStyle::PM_TextCursorWidth)
                                 , qApp->cursorFlashTime()
                                 , cursorFlashLimit);
    connect(&m_textCursor, SIGNAL(updateCursorDecoration(QRectF)), this, SLOT(slotUpdateCursorDecoration(QRectF)));

    Q_FOREACH(const QString name, SvgTextShortCuts::possibleActions()) {
        QAction *a = action(name);
        if(m_textCursor.registerPropertyAction(a, name)) {
            dbgTools << "registered" << name << a->shortcut();
        }
    }

    const QString glyphName = "svg_insert_special_character";
    QAction *glyphAction = action(glyphName);
    if (glyphAction) {
        m_textCursor.registerPropertyAction(glyphAction, glyphName);
        connect(&m_textCursor, SIGNAL(sigOpenGlyphPalette()), this, SLOT(showGlyphPalette()));
    }

    m_base_cursor = QCursor(QPixmap(":/tool_text_basic.xpm"), 7, 7);
    m_text_inline_horizontal = QCursor(QPixmap(":/tool_text_inline_horizontal.xpm"), 7, 7);
    m_text_inline_vertical = QCursor(QPixmap(":/tool_text_inline_vertical.xpm"), 7, 7);
    m_text_on_path = QCursor(QPixmap(":/tool_text_on_path.xpm"), 7, 7);
    m_text_in_shape = QCursor(QPixmap(":/tool_text_in_shape.xpm"), 7, 7);
    m_ibeam_horizontal = QCursor(QPixmap(":/tool_text_i_beam_horizontal.xpm"), 11, 11);
    m_ibeam_vertical = QCursor(QPixmap(":/tool_text_i_beam_vertical.xpm"), 11, 11);
    m_ibeam_horizontal_done = QCursor(QPixmap(":/tool_text_i_beam_horizontal_done.xpm"), 5, 11);
}

SvgTextTool::~SvgTextTool()
{
    if(m_editor) {
        m_editor->close();
    }
    if(m_glyphPalette) {
        m_glyphPalette->close();
    }
}

void SvgTextTool::activate(const QSet<KoShape *> &shapes)
{
    KoToolBase::activate(shapes);
    m_canvasConnections.addConnection(canvas()->selectedShapesProxy(), SIGNAL(selectionChanged()), this, SLOT(slotShapeSelectionChanged()));

    const KisCanvas2 *canvas2 = qobject_cast<const KisCanvas2 *>(this->canvas());
    if (canvas2) {
        canvas2->viewManager()->textPropertyManager()->setTextPropertiesInterface(m_textCursor.textPropertyInterface());
        QDockWidget *docker = canvas2->viewManager()->mainWindow()->dockWidget("TextProperties");
        if (docker && m_optionManager) {
            m_optionManager->setTextPropertiesOpen(docker->isVisible());
        }
    }

    useCursor(m_base_cursor);
    slotShapeSelectionChanged();

    repaintDecorations();
}

void SvgTextTool::deactivate()
{
    KoToolBase::deactivate();
    m_canvasConnections.clear();
    m_textCursor.setShape(nullptr);
    const KisCanvas2 *canvas2 = qobject_cast<const KisCanvas2 *>(this->canvas());
    if (canvas2) {
        canvas2->viewManager()->textPropertyManager()->setTextPropertiesInterface(nullptr);
    }
    // Exiting text editing mode is handled by requestStrokeEnd

    m_hoveredShapeHighlightRect = QPainterPath();

    repaintDecorations();
}

KisPopupWidgetInterface *SvgTextTool::popupWidget()
{
    return nullptr;
}

QVariant SvgTextTool::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (canvas()) {
        return m_textCursor.inputMethodQuery(query);
    } else {
        return KoToolBase::inputMethodQuery(query);
    }
}

void SvgTextTool::inputMethodEvent(QInputMethodEvent *event)
{
    m_textCursor.inputMethodEvent(event);
}

QWidget *SvgTextTool::createOptionWidget()
{
    KisQQuickWidget *optionWidget = new KisQQuickWidget();
    optionWidget->setMinimumWidth(100);
    optionWidget->setMinimumHeight(100);

    optionWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    optionWidget->setSource(QUrl("qrc:/SvgTextToolOptions.qml"));

    m_optionManager->setShowDebug(debugEnabled());
    if (optionWidget->errors().isEmpty()) {
        optionWidget->rootObject()->setProperty("manager", QVariant::fromValue(m_optionManager.data()));
        optionWidget->connectMinimumHeightToRootObject();
    } else {
        qWarning() << optionWidget->errors();
    }


    connect(m_optionManager.data(), SIGNAL(openTextEditor()), SLOT(showEditor()));
    connect(m_optionManager.data(), SIGNAL(openGlyphPalette()), SLOT(showGlyphPalette()));

    connect(m_optionManager.data(), SIGNAL(convertTextType(int)), SLOT(slotConvertType(int)));
    connect(m_optionManager->optionsModel(), SIGNAL(useVisualBidiCursorChanged(bool)), this, SLOT(slotUpdateVisualCursor()));
    connect(m_optionManager->optionsModel(), SIGNAL(pasteRichtTextByDefaultChanged(bool)), this, SLOT(slotUpdateTextPasteBehaviour()));
    const KisCanvas2 *canvas2 = qobject_cast<const KisCanvas2 *>(this->canvas());
    if (canvas2 && canvas2->viewManager()->mainWindow()) {
        QDockWidget *docker = canvas2->viewManager()->mainWindow()->dockWidget("TextProperties");
        m_optionManager->setShowTextPropertyButton((docker));
        if (docker) {
            optionWidget->setPalette(docker->palette());
            connect(m_optionManager.data(), SIGNAL(openTextPropertiesDocker(bool)), docker, SLOT(setVisible(bool)));
        }
    }
    slotUpdateVisualCursor();
    slotUpdateTextPasteBehaviour();
    slotTextTypeUpdated();


    return optionWidget;
}

KoSelection *SvgTextTool::koSelection() const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(canvas(), 0);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(canvas()->selectedShapesProxy(), 0);

    return canvas()->selectedShapesProxy()->selection();
}

KoSvgTextShape *SvgTextTool::selectedShape() const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(canvas(), 0);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(canvas()->selectedShapesProxy(), 0);

    QList<KoShape*> shapes = koSelection()->selectedEditableShapes();
    if (shapes.isEmpty()) return 0;

    KoSvgTextShape *textShape = dynamic_cast<KoSvgTextShape*>(shapes.first());

    return textShape;
}

void SvgTextTool::showEditor()
{
    KoSvgTextShape *shape = selectedShape();
    if (!shape) return;

    if (!m_editor) {
        m_editor = new SvgTextEditor(QApplication::activeWindow());
        m_editor->setWindowTitle(i18nc("@title:window", "Krita - Edit Text"));
        m_editor->setWindowModality(Qt::ApplicationModal);
        m_editor->setAttribute( Qt::WA_QuitOnClose, false );

        connect(m_editor, SIGNAL(textUpdated(KoSvgTextShape*,QString,QString)), SLOT(textUpdated(KoSvgTextShape*,QString,QString)));
        connect(m_editor, SIGNAL(textEditorClosed()), SLOT(slotTextEditorClosed()));

        m_editor->activateWindow(); // raise on creation only
    }
    if (!m_editor->isVisible()) {
        m_editor->setInitialShape(shape);
#ifdef Q_OS_ANDROID
        // for window manager
        m_editor->setWindowFlags(Qt::Dialog);
        m_editor->menuBar()->setNativeMenuBar(false);
#endif
        m_editor->show();
    }
}

void SvgTextTool::textUpdated(KoSvgTextShape *shape, const QString &svg, const QString &defs)
{
    SvgTextChangeCommand *cmd = new SvgTextChangeCommand(shape, svg, defs);
    canvas()->addCommand(cmd);
}

void SvgTextTool::showGlyphPalette()
{
    if (!m_glyphPalette) {
        m_glyphPalette = new GlyphPaletteDialog(QApplication::activeWindow());
        m_glyphPalette->setAttribute( Qt::WA_QuitOnClose, false );

        connect(&m_textCursor, SIGNAL(selectionChanged()), this, SLOT(updateGlyphPalette()));
        connect(m_glyphPalette, SIGNAL(signalInsertRichText(KoSvgTextShape*, bool)), this, SLOT(insertRichText(KoSvgTextShape*, bool)));

        m_glyphPalette->activateWindow();
    }
    if (!m_glyphPalette->isVisible()) {
        m_glyphPalette->show();
        updateGlyphPalette();
    }
}

void SvgTextTool::updateGlyphPalette()
{
    if (m_glyphPalette && m_glyphPalette->isVisible()) {
        QString grapheme = QString();
        if (m_textCursor.shape()) {
            int pos = m_textCursor.getPos();
            int pos2 = pos > 0? m_textCursor.shape()->posLeft(pos, false): m_textCursor.shape()->posRight(pos, false);
            int start = m_textCursor.shape()->indexForPos(qMin(pos, pos2));
            int end   = m_textCursor.shape()->indexForPos(qMax(pos, pos2));
            grapheme = m_textCursor.shape()->plainText().mid(start, end-start);
        }
        m_glyphPalette->setGlyphModelFromProperties(m_textCursor.currentTextProperties(), grapheme);
    }
}

void SvgTextTool::insertRichText(KoSvgTextShape *richText, bool replaceLastGlyph)
{
    if (replaceLastGlyph) {
        m_textCursor.setPos(m_textCursor.getPos(), m_textCursor.getPos());
        m_textCursor.moveCursor(m_textCursor.getPos() == 0? SvgTextCursor::MoveNextChar : SvgTextCursor::MovePreviousChar, false);
    }
    m_textCursor.insertRichText(richText);
}

void SvgTextTool::slotTextEditorClosed()
{
    // change tools to the shape selection tool when we close the text editor to allow moving and further editing of the object.
    // most of the time when we edit text, the shape selection tool is where we left off anyway
    KoToolManager::instance()->switchToolRequested("InteractionTool");
}

QString SvgTextTool::generateDefs(const KoSvgTextProperties &properties)
{
    QStringList propStrings;
    QMap<QString, QString> paraProps = properties.convertParagraphProperties();
    for (auto it = paraProps.constBegin(); it != paraProps.constEnd(); it++) {
        propStrings.append(QString("%1: %2;").arg(it.key()).arg(it.value()));
    }
    paraProps = properties.convertToSvgTextAttributes();
    for (auto it = paraProps.constBegin(); it != paraProps.constEnd(); it++) {
        propStrings.append(QString("%1: %2;").arg(it.key()).arg(it.value()));
    }

    return QString("<defs>\n <style>\n  text {\n   %1\n  }\n </style>\n</defs>").arg(propStrings.join("\n   "));
}

KoSvgTextProperties SvgTextTool::propertiesForNewText() const
{
    const bool useCurrent = m_optionManager->optionsModel()->useCurrentTextProperties();
    const QString presetName = m_optionManager->optionsModel()->cssStylePresetName();

    KoSvgTextProperties props;
    if (useCurrent || presetName.isEmpty()) {
        KoSvgTextPropertyData textData = canvas()->resourceManager()->resource(KoCanvasResource::SvgTextPropertyData).value<KoSvgTextPropertyData>();
        props = textData.commonProperties;
    } else {
        KisAllResourcesModel *model = KisResourceModelProvider::resourceModel(ResourceType::CssStyles);
        QVector<KoResourceSP> res = model->resourcesForName(presetName);
        if (res.first()) {
            KoCssStylePresetSP style = res.first().staticCast<KoCssStylePreset>();
            if (style) {
                props = style->properties();
            }
        }
    }

    QColor fontColor = (canvas()->resourceManager()->isUsingOtherColor()
                ? canvas()->resourceManager()->backgroundColor()
                : canvas()->resourceManager()->foregroundColor()).toQColor();
    QSharedPointer<KoColorBackground> bg(new KoColorBackground());
    bg->setColor(fontColor);
    KoSvgText::BackgroundProperty bgProp(bg);
    props.setProperty(KoSvgTextProperties::FillId, QVariant::fromValue(bgProp));
    return props;
}

void SvgTextTool::slotShapeSelectionChanged()
{
    QList<KoShape *> shapes = koSelection()->selectedEditableShapes();
    if (shapes.size() == 1) {
        KoSvgTextShape *textShape = selectedShape();
        if (!textShape) {
            koSelection()->deselectAll();
            return;
        }
    } else if (shapes.size() > 1) {
        KoSvgTextShape *foundTextShape = nullptr;

        Q_FOREACH (KoShape *shape, shapes) {
            KoSvgTextShape *textShape = dynamic_cast<KoSvgTextShape*>(shape);
            if (textShape) {
                foundTextShape = textShape;
                break;
            }
        }

        koSelection()->deselectAll();
        if (foundTextShape) {
            koSelection()->select(foundTextShape);
        }
        return;
    }
    KoSvgTextShape *const shape = selectedShape();
    if (shape != m_textCursor.shape()) {
        m_textCursor.setShape(shape);
        if (shape) {
            setTextMode(true);
        } else {
            setTextMode(false);
        }
    }
    slotTextTypeUpdated();
}

void SvgTextTool::copy() const
{
    m_textCursor.copy();
}

void SvgTextTool::deleteSelection()
{
    m_textCursor.removeSelection();
}

bool SvgTextTool::paste()
{
    return m_textCursor.paste();
}

bool SvgTextTool::hasSelection()
{
    return m_textCursor.hasSelection();
}

bool SvgTextTool::selectAll()
{
    m_textCursor.moveCursor(SvgTextCursor::ParagraphStart, true);
    m_textCursor.moveCursor(SvgTextCursor::ParagraphEnd, false);
    return true;
}

void SvgTextTool::deselect()
{
    m_textCursor.deselectText();
}

KoToolSelection *SvgTextTool::selection()
{
    return &m_textCursor;
}

void SvgTextTool::requestStrokeEnd()
{
    if (!isActivated()) return;
    if (!m_textCursor.isAddingCommand() && !m_strategyAddingCommand) {
        if (m_interactionStrategy) {
            m_dragging = DragMode::None;
            m_interactionStrategy->cancelInteraction();
            m_interactionStrategy = nullptr;
            useCursor(Qt::ArrowCursor);
        } else if (isInTextMode()) {
            canvas()->shapeManager()->selection()->deselectAll();
        }
    }
}

void SvgTextTool::requestStrokeCancellation()
{
    /**
     * Doing nothing, since these signals come on undo/redo actions
     * in the mainland undo stack, which we manipulate while editing
     * text
     */
}

void SvgTextTool::slotUpdateCursorDecoration(QRectF updateRect)
{
    if (canvas()) {
        canvas()->updateCanvas(updateRect);
    }
}

void SvgTextTool::slotConvertType(int index) {
    if (selectedShape()) {
        if (index == selectedShape()->textType()) return;
        SvgConvertTextTypeCommand::ConversionType type = SvgConvertTextTypeCommand::ToPreFormatted;
        if (index == KoSvgTextShape::InlineWrap) {
            type = SvgConvertTextTypeCommand::ToInlineSize;
        } else if (index == KoSvgTextShape::PrePositionedText) {
            type = SvgConvertTextTypeCommand::ToCharTransforms;
        }
        KUndo2Command *parentCommand = new KUndo2Command();
        new KoKeepShapesSelectedCommand({selectedShape()}, {}, canvas()->selectedShapesProxy(), KisCommandUtils::FlipFlopCommand::State::INITIALIZING, parentCommand);
        SvgConvertTextTypeCommand *cmd = new SvgConvertTextTypeCommand(selectedShape(), type, m_textCursor.getPos(), parentCommand);
        parentCommand->setText(cmd->text());
        new KoKeepShapesSelectedCommand({}, {selectedShape()}, canvas()->selectedShapesProxy(), KisCommandUtils::FlipFlopCommand::State::FINALIZING, parentCommand);
        canvas()->addCommand(parentCommand);
        slotTextTypeUpdated();
    }
}

void SvgTextTool::slotUpdateVisualCursor()
{
    m_textCursor.setVisualMode(m_optionManager->optionsModel()->useVisualBidiCursor());
}

void SvgTextTool::slotUpdateTextPasteBehaviour()
{
    m_textCursor.setPasteRichTextByDefault(m_optionManager->optionsModel()->pasteRichtTextByDefault());
}

void SvgTextTool::slotTextTypeUpdated()
{
    KoSvgTextShape *shape = selectedShape();
    if (shape && m_optionManager) {
        m_optionManager->convertToTextType(int(shape->textType()));
    } else {
        m_optionManager->convertToTextType(-1);
    }
}

QRectF SvgTextTool::decorationsRect() const
{
    QRectF rect;
    KoSvgTextShape *const shape = selectedShape();
    if (shape) {
        rect |= shape->boundingRect();

        const QPointF anchor = shape->absoluteTransformation().map(QPointF());
        rect |= kisGrowRect(QRectF(anchor, anchor), handleRadius());

        qreal pxlToPt = canvas()->viewConverter()->viewToDocumentX(1.0);
        qreal length = (INLINE_SIZE_DASHES_PATTERN_A + INLINE_SIZE_DASHES_PATTERN_B) * INLINE_SIZE_DASHES_PATTERN_LENGTH;

        if (std::optional<InlineSizeInfo> info = InlineSizeInfo::fromShape(shape, length * pxlToPt)) {
            rect |= kisGrowRect(info->boundingRect(), handleRadius() * 2);
        }

        if (canvas()->snapGuide()->isSnapping()) {
            rect |= canvas()->snapGuide()->boundingRect();
        }
    }

    rect |= m_hoveredShapeHighlightRect.boundingRect();

    return rect;
}

void SvgTextTool::paint(QPainter &gc, const KoViewConverter &converter)
{
    if (!isActivated()) return;

    if (m_dragging == DragMode::Create) {
        m_interactionStrategy->paint(gc, converter);
    }

    KoSvgTextShape *shape = selectedShape();
    if (shape) {
        KisHandlePainterHelper handlePainter =
            KoShape::createHandlePainterHelperView(&gc, shape, converter, handleRadius(), decorationThickness());

        if (m_dragging != DragMode::InlineSizeHandle && m_dragging != DragMode::Move) {
            handlePainter.setHandleStyle(KisHandleStyle::primarySelection());
            QPainterPath path;
            path.addRect(shape->outlineRect());
            handlePainter.drawPath(path);
        }


        qreal pxlToPt = canvas()->viewConverter()->viewToDocumentX(1.0);
        qreal length = (INLINE_SIZE_DASHES_PATTERN_A + INLINE_SIZE_DASHES_PATTERN_B) * INLINE_SIZE_DASHES_PATTERN_LENGTH;
        if (std::optional<InlineSizeInfo> info = InlineSizeInfo::fromShape(shape, length * pxlToPt)) {
            handlePainter.setHandleStyle(KisHandleStyle::secondarySelection());
            handlePainter.drawConnectionLine(info->baselineLineLocal());

            if (m_highlightItem == HighlightItem::InlineSizeStartHandle) {
                handlePainter.setHandleStyle(m_dragging == DragMode::InlineSizeHandle? KisHandleStyle::partiallyHighlightedPrimaryHandles()
                                                                                     : KisHandleStyle::highlightedPrimaryHandles());
            }
            QVector<qreal> dashPattern = {INLINE_SIZE_DASHES_PATTERN_A, INLINE_SIZE_DASHES_PATTERN_B};
            handlePainter.drawHandleLine(info->startLineLocal());
            handlePainter.drawHandleLine(info->startLineDashes(), INLINE_SIZE_HANDLE_THICKNESS, dashPattern, INLINE_SIZE_DASHES_PATTERN_A);

            handlePainter.setHandleStyle(KisHandleStyle::secondarySelection());
            if (m_highlightItem == HighlightItem::InlineSizeEndHandle) {
                handlePainter.setHandleStyle(m_dragging == DragMode::InlineSizeHandle? KisHandleStyle::partiallyHighlightedPrimaryHandles()
                                                                                     : KisHandleStyle::highlightedPrimaryHandles());
            }
            handlePainter.drawHandleLine(info->endLineLocal());
            handlePainter.drawHandleLine(info->endLineDashes(), INLINE_SIZE_HANDLE_THICKNESS, dashPattern, INLINE_SIZE_DASHES_PATTERN_A);
        }

        if (m_highlightItem == HighlightItem::MoveBorder) {
            handlePainter.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());
        } else {
            handlePainter.setHandleStyle(KisHandleStyle::primarySelection());
        }
        handlePainter.drawHandleCircle(QPointF(), KoToolBase::handleRadius() * 0.75);
    }

    gc.setTransform(converter.documentToView(), true);
    {
        KisHandlePainterHelper handlePainter(&gc, handleRadius(), decorationThickness());
        if (!m_hoveredShapeHighlightRect.isEmpty()) {
            handlePainter.setHandleStyle(KisHandleStyle::highlightedPrimaryHandlesWithSolidOutline());
            QPainterPath path;
            path.addPath(m_hoveredShapeHighlightRect);
            handlePainter.drawPath(path);
        }
    }
    if (shape) {
            m_textCursor.paintDecorations(gc, qApp->palette().color(QPalette::Highlight), decorationThickness());
    }
    if (m_interactionStrategy) {
        gc.save();
        canvas()->snapGuide()->paint(gc, converter);
        gc.restore();
    }

    // Paint debug outline
    if (debugEnabled() && shape) {
        gc.save();
        using Element = KoSvgTextShape::DebugElement;
        KoSvgTextShape::DebugElements el{};
        if (m_optionManager->showCharacterDebug()) {
            el |= Element::CharBbox;
        }
        if (m_optionManager->showLineDebug()) {
            el |= Element::LineBox;
        }

        gc.setTransform(shape->absoluteTransformation(), true);
        shape->paintDebug(gc, el);
        gc.restore();
    }
}

void SvgTextTool::mousePressEvent(KoPointerEvent *event)
{
    KoSvgTextShape *selectedShape = this->selectedShape();

    if (selectedShape) {
        if (m_highlightItem == HighlightItem::MoveBorder) {
            m_interactionStrategy.reset(new SvgMoveTextStrategy(this, selectedShape, event->point));
            m_dragging = DragMode::Move;
            event->accept();
            return;
        } else if (m_highlightItem == HighlightItem::InlineSizeEndHandle) {
            m_interactionStrategy.reset(new SvgInlineSizeChangeStrategy(this, selectedShape, event->point, false));
            m_dragging = DragMode::InlineSizeHandle;
            event->accept();
            return;
        }  else if (m_highlightItem == HighlightItem::InlineSizeStartHandle) {
            m_interactionStrategy.reset(new SvgInlineSizeChangeStrategy(this, selectedShape, event->point, true));
            m_dragging = DragMode::InlineSizeHandle;
            event->accept();
            return;
        }
    }

    KoSvgTextShape *hoveredShape = dynamic_cast<KoSvgTextShape *>(canvas()->shapeManager()->shapeAt(event->point));
    QString shapeType;
    QPainterPath hoverPath = KisToolUtils::shapeHoverInfoCrossLayer(canvas(), event->point, shapeType);
    bool crossLayerPossible = !hoverPath.isEmpty() && shapeType == KoSvgTextShape_SHAPEID;

    if (!selectedShape && !hoveredShape && !crossLayerPossible) {
        QPointF point = canvas()->snapGuide()->snap(event->point, event->modifiers());
        m_interactionStrategy.reset(new SvgCreateTextStrategy(this, point));
        m_dragging = DragMode::Create;
        event->accept();
    } else if (hoveredShape) {
        if (hoveredShape != selectedShape) {
            canvas()->shapeManager()->selection()->deselectAll();
            canvas()->shapeManager()->selection()->select(hoveredShape);
            m_hoveredShapeHighlightRect = QPainterPath();
        }
        m_interactionStrategy.reset(new SvgSelectTextStrategy(this, &m_textCursor, event->point));
        m_dragging = DragMode::Select;
        event->accept();
    } else if (crossLayerPossible) {
        if (KisToolUtils::selectShapeCrossLayer(canvas(), event->point, KoSvgTextShape_SHAPEID)) {
            m_interactionStrategy.reset(new SvgSelectTextStrategy(this, &m_textCursor, event->point));
            m_dragging = DragMode::Select;
            m_hoveredShapeHighlightRect = QPainterPath();
        } else {
            canvas()->shapeManager()->selection()->deselectAll();
        }
        event->accept();
    } else { // if there's a selected shape but no hovered shape...
        canvas()->shapeManager()->selection()->deselectAll();
        event->accept();
    }

    repaintDecorations();
}

static inline Qt::CursorShape angleToCursor(const QVector2D unit)
{
    constexpr float SIN_PI_8 = 0.382683432;
    if (unit.y() < SIN_PI_8 && unit.y() > -SIN_PI_8) {
        return Qt::SizeHorCursor;
    } else if (unit.x() < SIN_PI_8 && unit.x() > -SIN_PI_8) {
        return Qt::SizeVerCursor;
    } else if ((unit.x() > 0 && unit.y() > 0) || (unit.x() < 0 && unit.y() < 0)) {
        return Qt::SizeFDiagCursor;
    } else {
        return Qt::SizeBDiagCursor;
    }
}

static inline Qt::CursorShape lineToCursor(const QLineF line, const KoCanvasBase *const canvas)
{
    const KisCanvas2 *const canvas2 = qobject_cast<const KisCanvas2 *>(canvas);
    KIS_ASSERT(canvas2);
    const KisCoordinatesConverter *const converter = canvas2->coordinatesConverter();
    QLineF wdgLine = converter->flakeToWidget(line);
    return angleToCursor(QVector2D(wdgLine.p2() - wdgLine.p1()).normalized());
}

void SvgTextTool::mouseMoveEvent(KoPointerEvent *event)
{
    m_lastMousePos = event->point;
    m_hoveredShapeHighlightRect = QPainterPath();


    if (m_interactionStrategy) {
        m_interactionStrategy->handleMouseMove(event->point, event->modifiers());
        if (m_dragging == DragMode::Create) {
            SvgCreateTextStrategy *c = dynamic_cast<SvgCreateTextStrategy*>(m_interactionStrategy.get());
            if (c && c->draggingInlineSize()) {
                if (this->writingMode() == KoSvgText::HorizontalTB) {
                    useCursor(m_text_inline_horizontal);
                } else {
                    useCursor(m_text_inline_vertical);
                }
            } else {
                useCursor(m_base_cursor);
            }
        } else if (m_dragging == DragMode::Select && this->selectedShape()) {
            KoSvgTextShape *const selectedShape = this->selectedShape();
            // Todo: replace with something a little less hacky.
            if (selectedShape->writingMode() == KoSvgText::HorizontalTB) {
                useCursor(m_ibeam_horizontal);
            } else {
                useCursor(m_ibeam_vertical);
            }
        }
        event->accept();
    } else {
        m_highlightItem = HighlightItem::None;
        KoSvgTextShape *const selectedShape = this->selectedShape();
        QCursor cursor = m_base_cursor;
        if (selectedShape) {
            cursor = m_ibeam_horizontal_done;
            const qreal sensitivity = grabSensitivityInPt();

            if (std::optional<InlineSizeInfo> info = InlineSizeInfo::fromShape(selectedShape)) {
                const QPolygonF zone = info->endLineGrabRect(sensitivity);
                const QPolygonF startZone = info->startLineGrabRect(sensitivity);
                if (zone.containsPoint(event->point, Qt::OddEvenFill)) {
                    m_highlightItem = HighlightItem::InlineSizeEndHandle;
                    cursor = lineToCursor(info->baselineLine(), canvas());
                } else if (startZone.containsPoint(event->point, Qt::OddEvenFill)){
                    m_highlightItem = HighlightItem::InlineSizeStartHandle;
                    cursor = lineToCursor(info->baselineLine(), canvas());
                }
            }

            if (m_highlightItem == HighlightItem::None) {
                const QPolygonF textOutline = selectedShape->absoluteTransformation().map(selectedShape->outlineRect());
                const QPolygonF moveBorderRegion = selectedShape->absoluteTransformation().map(kisGrowRect(selectedShape->outlineRect(),
                                                                                                           sensitivity * 2));
                if (moveBorderRegion.containsPoint(event->point, Qt::OddEvenFill) && !textOutline.containsPoint(event->point, Qt::OddEvenFill)) {
                    m_highlightItem = HighlightItem::MoveBorder;
                    cursor = Qt::SizeAllCursor;
                }
            }
        }

        QString shapeType;
        bool isHorizontal = true;
        const KoSvgTextShape *hoveredShape = dynamic_cast<KoSvgTextShape *>(canvas()->shapeManager()->shapeAt(event->point));
        QPainterPath hoverPath = KisToolUtils::shapeHoverInfoCrossLayer(canvas(), event->point, shapeType, &isHorizontal);
        if (selectedShape && selectedShape == hoveredShape && m_highlightItem == HighlightItem::None) {
            if (selectedShape->writingMode() == KoSvgText::HorizontalTB) {
                cursor = m_ibeam_horizontal;
            } else {
                cursor = m_ibeam_vertical;
            }
        } else if (hoveredShape) {
            if (!hoveredShape->shapesInside().isEmpty()) {
                QPainterPath paths;
                Q_FOREACH(KoShape *s, hoveredShape->shapesInside()) {
                    KoPathShape *path = dynamic_cast<KoPathShape *>(s);
                    if (path) {
                        paths.addPath(hoveredShape->absoluteTransformation().map(path->absoluteTransformation().map(path->outline())));
                    }
                }
                if (!paths.isEmpty()) {
                    m_hoveredShapeHighlightRect = paths;
                }
            } else {
                m_hoveredShapeHighlightRect.addRect(hoveredShape->boundingRect());
            }
            if (hoveredShape->writingMode() == KoSvgText::HorizontalTB) {
                cursor = m_ibeam_horizontal;
            } else {
                cursor = m_ibeam_vertical;
            }
        } else if (!hoverPath.isEmpty() && shapeType == KoSvgTextShape_SHAPEID && m_highlightItem == HighlightItem::None) {
            m_hoveredShapeHighlightRect = hoverPath;
            if (isHorizontal) {
                cursor = m_ibeam_horizontal;
            } else {
                cursor = m_ibeam_vertical;
            }
        }
#if 0
        /// Commenting this out until we have a good idea of how we want to tackle the text and shape to put them on.
           else if(m_highlightItem == HighlightItem::None) {
            KoPathShape *shape = dynamic_cast<KoPathShape *>(canvas()->shapeManager()->shapeAt(event->point));
            if (shape) {
                if (shape->subpathCount() > 0) {
                    if (shape->isClosedSubpath(0)) {
                        cursor = m_text_in_shape;
                    }
                }
                KoPathSegment segment = segmentAtPoint(event->point, shape, handleGrabRect(event->point));
                if (segment.isValid()) {
                    cursor = m_text_on_path;
                }
                m_hoveredShapeHighlightRect.addPath(shape->absoluteTransformation().map(shape->outline()));
            } else {
                m_hoveredShapeHighlightRect = QPainterPath();
            }
        }
#endif
        useCursor(cursor);
        event->ignore();
    }

    repaintDecorations();
}

void SvgTextTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if (m_interactionStrategy) {
        m_interactionStrategy->finishInteraction(event->modifiers());
        KUndo2Command *const command = m_interactionStrategy->createCommand();
        if (command) {
            m_strategyAddingCommand = true;
            canvas()->addCommand(command);
            m_strategyAddingCommand = false;
        }
        m_interactionStrategy = nullptr;
        if (m_dragging != DragMode::Select) {
            useCursor(m_base_cursor);
        }
        m_dragging = DragMode::None;
        event->accept();
    } else {
        useCursor(m_base_cursor);
    }
    event->accept();
}

void SvgTextTool::keyPressEvent(QKeyEvent *event)
{
    if (m_interactionStrategy
        && (event->key() == Qt::Key_Control || event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift
            || event->key() == Qt::Key_Meta)) {
        m_interactionStrategy->handleMouseMove(m_lastMousePos, event->modifiers());
        event->accept();
        return;
    } else if (event->key() == Qt::Key_Escape) {
        requestStrokeEnd();
    } else if (selectedShape()) {
        m_textCursor.keyPressEvent(event);
    }

    event->ignore();
}

void SvgTextTool::keyReleaseEvent(QKeyEvent *event)
{
    if (m_interactionStrategy
        && (event->key() == Qt::Key_Control || event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift
            || event->key() == Qt::Key_Meta)) {
        m_interactionStrategy->handleMouseMove(m_lastMousePos, event->modifiers());
        event->accept();
    } else {
        event->ignore();
    }
}

void SvgTextTool::focusInEvent(QFocusEvent *event)
{
    m_textCursor.focusIn();
    event->accept();
}

void SvgTextTool::focusOutEvent(QFocusEvent *event)
{
    m_textCursor.focusOut();
    event->accept();
}

void SvgTextTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    if (canvas()->shapeManager()->shapeAt(event->point) != selectedShape()) {
        event->ignore(); // allow the event to be used by another
        return;
    } else {
        m_textCursor.setPosToPoint(event->point, true);
        m_textCursor.moveCursor(SvgTextCursor::MoveWordLeft, true);
        m_textCursor.moveCursor(SvgTextCursor::MoveWordRight, false);
    }
    const QRectF updateRect = std::exchange(m_hoveredShapeHighlightRect, QPainterPath()).boundingRect();
    canvas()->updateCanvas(kisGrowRect(updateRect, 100));
    event->accept();
}

void SvgTextTool::mouseTripleClickEvent(KoPointerEvent *event)
{
    if (canvas()->shapeManager()->shapeAt(event->point) == selectedShape()) {
        // TODO: Consider whether we want to use sentence based selection instead:
        // QTextBoundaryFinder allows us to find sentences if necessary.
        m_textCursor.moveCursor(SvgTextCursor::ParagraphStart, true);
        m_textCursor.moveCursor(SvgTextCursor::ParagraphEnd, false);
        event->accept();
    }
}

qreal SvgTextTool::grabSensitivityInPt() const
{
    const int sensitivity = grabSensitivity();
    return canvas()->viewConverter()->viewToDocumentX(sensitivity);
}

KoSvgText::WritingMode SvgTextTool::writingMode() const
{
    KoSvgTextProperties props = propertiesForNewText();
    return KoSvgText::WritingMode(props.propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
}

