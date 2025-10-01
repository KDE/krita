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
#include "KoSvgConvertTextTypeCommand.h"
#include "SvgTextShortCuts.h"
#include "SvgTextToolOptionsModel.h"
#include "SvgTextTypeSettingStrategy.h"
#include "SvgTextChangeTransformsOnRange.h"

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

#ifdef Q_OS_ANDROID
#include <QMenuBar>
#endif


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
    , m_textOutlineHelper(new KoSvgTextShapeOutlineHelper(canvas))
{
     // TODO: figure out whether we should use system config for this, Windows and GTK have values for it, but Qt and MacOS don't(?).
    const int cursorFlashLimit = 5000;
    const bool enableCursorWithSelection = QApplication::style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected);
    m_textCursor.setCaretSetting(QApplication::style()->pixelMetric(QStyle::PM_TextCursorWidth)
                                 , qApp->cursorFlashTime()
                                 , cursorFlashLimit
                                 , enableCursorWithSelection);
    connect(&m_textCursor, SIGNAL(updateCursorDecoration(QRectF)), this, SLOT(slotUpdateCursorDecoration(QRectF)));

    Q_FOREACH(const QString name, SvgTextShortCuts::possibleActions()) {
        QAction *a = action(name);
        if(m_textCursor.registerPropertyAction(a, name)) {
            dbgTools << "registered" << name << a->shortcut();
        }
    }

    const QStringList extraActions = {
        "svg_insert_special_character",
        "svg_paste_rich_text",
        "svg_paste_plain_text",
        "svg_remove_transforms_from_range",
        "svg_clear_formatting"
    };
    connect(&m_textCursor, SIGNAL(sigOpenGlyphPalette()), this, SLOT(showGlyphPalette()));
    Q_FOREACH (const QString name, extraActions) {
        QAction *a = action(name);
        if (a) {
            if(!m_textCursor.registerPropertyAction(a, name)) {
                qWarning() << "could not register" << name << a->shortcut();
            }
        }
    }

    m_textTypeSignalsMapper.reset(new KisSignalMapper(this));
    addMappedAction(m_textTypeSignalsMapper.data(), "text_type_preformatted", KoSvgTextShape::PreformattedText);
    addMappedAction(m_textTypeSignalsMapper.data(), "text_type_inline_wrap", KoSvgTextShape::InlineWrap);
    addMappedAction(m_textTypeSignalsMapper.data(), "text_type_pre_positioned", KoSvgTextShape::PrePositionedText);

    m_typeSettingMovementMapper.reset(new KisSignalMapper(this));
    addMappedAction(m_typeSettingMovementMapper.data(), "svg_type_setting_move_selection_start_down_1_px", Qt::Key_Down);
    addMappedAction(m_typeSettingMovementMapper.data(), "svg_type_setting_move_selection_start_up_1_px", Qt::Key_Up);
    addMappedAction(m_typeSettingMovementMapper.data(), "svg_type_setting_move_selection_start_left_1_px", Qt::Key_Left);
    addMappedAction(m_typeSettingMovementMapper.data(), "svg_type_setting_move_selection_start_right_1_px", Qt::Key_Right);

    m_textOutlineHelper->setDrawBoundingRect(false);
    m_textOutlineHelper->setDrawTextWrappingArea(true);

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

    KisCanvas2 *canvas2 = qobject_cast<KisCanvas2 *>(this->canvas());
    if (canvas2) {
        canvas2->setTextShapeManagerEnabled(nullptr);
        canvas2->viewManager()->textPropertyManager()->setTextPropertiesInterface(m_textCursor.textPropertyInterface());
        QDockWidget *docker = canvas2->viewManager()->mainWindow()->dockWidget("TextProperties");
        if (docker && m_optionManager) {
            m_optionManager->setTextPropertiesOpen(docker->isVisible());
        }
    }

    connect(m_textTypeSignalsMapper.data(), SIGNAL(mapped(int)), this, SLOT(slotConvertType(int)));
    connect(m_typeSettingMovementMapper.data(), SIGNAL(mapped(int)), this, SLOT(slotMoveTextSelection(int)));

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
    disconnect(m_textTypeSignalsMapper.data(), 0, this, 0);
    disconnect(m_typeSettingMovementMapper.data(), 0, this, 0);

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
    connect(m_optionManager.data(), SIGNAL(typeSettingModeChanged()), SLOT(slotUpdateTypeSettingMode()));
    connect(m_optionManager->optionsModel(), SIGNAL(useVisualBidiCursorChanged(bool)), this, SLOT(slotUpdateVisualCursor()));
    connect(m_optionManager->optionsModel(), SIGNAL(pasteRichtTextByDefaultChanged(bool)), this, SLOT(slotUpdateTextPasteBehaviour()));
    const KisCanvas2 *canvas2 = qobject_cast<const KisCanvas2 *>(this->canvas());
    if (canvas2 && canvas2->viewManager()->mainWindow()) {
        QDockWidget *docker = canvas2->viewManager()->mainWindow()->dockWidget("TextProperties");
        m_optionManager->setShowTextPropertyButton((docker));
        if (docker) {
            optionWidget->setPalette(docker->palette());
            m_optionManager->setTextPropertiesOpen(docker->isVisible());
            connect(m_optionManager.data(), &SvgTextToolOptionsManager::openTextPropertiesDocker, [docker](){
                        docker->setVisible(!docker->isVisible());
                    });
            // Once we have docker toggling actions, we should revisit this.
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
            qreal dpi = 72;
            KisCanvas2 *canvas2 = qobject_cast<KisCanvas2 *>(this->canvas());
            if (canvas2) {
                dpi = canvas2->image()->xRes() * 72.0;
            }
            if (style) {
                props = style->properties(dpi, true);
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
        KoSvgTextShape::TextType type = KoSvgTextShape::TextType(index);
        KUndo2Command *parentCommand = new KUndo2Command();
        new KoKeepShapesSelectedCommand({selectedShape()}, {}, canvas()->selectedShapesProxy(), KisCommandUtils::FlipFlopCommand::State::INITIALIZING, parentCommand);
        KoSvgConvertTextTypeCommand *cmd = new KoSvgConvertTextTypeCommand(selectedShape(), type, m_textCursor.getPos(), parentCommand);
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
    if (m_optionManager) {
        if (shape) {
            m_optionManager->convertToTextType(int(shape->textType()));
            action("text_type_preformatted")->setEnabled(true);
            action("text_type_pre_positioned")->setEnabled(true);
            action("text_type_inline_wrap")->setEnabled(true);
            action("text_type_preformatted")->setChecked(shape->textType() == KoSvgTextShape::PreformattedText);
            action("text_type_pre_positioned")->setChecked(shape->textType() == KoSvgTextShape::PrePositionedText);
            action("text_type_inline_wrap")->setChecked(shape->textType() == KoSvgTextShape::InlineWrap);

        } else {
            m_optionManager->convertToTextType(-1);
            action("text_type_preformatted")->setEnabled(false);
            action("text_type_pre_positioned")->setEnabled(false);
            action("text_type_inline_wrap")->setEnabled(false);
        }
        Q_FOREACH(QObject *obj, m_typeSettingMovementMapper->children()) {
            QAction *a = qobject_cast<QAction*>(obj);
            if (a && shape) {
                a->setEnabled(m_optionManager->typeSettingMode());
            }
        }
    }
}

void SvgTextTool::slotMoveTextSelection(int index)
{
    KoSvgTextShape *shape = selectedShape();
    if (!shape) return;
    QPointF offset;
    // test type setting mode.
    if (index == Qt::Key_Down) {
        offset = QPointF(0, 1);
    } else if (index == Qt::Key_Up) {
        offset = QPointF(0, -1);
    } else if (index == Qt::Key_Right) {
        offset = QPointF(-1, 0);
    } else if (index == Qt::Key_Left) {
        offset = QPointF(1, 0);
    } else {
        return;
    }
    const KisCanvas2 *canvas2 = qobject_cast<const KisCanvas2 *>(this->canvas());
    if (canvas2) {
        offset = canvas2->coordinatesConverter()->imageToDocumentTransform().map(offset);
    }
    KUndo2Command *parentCommand = new KUndo2Command();
    new KoKeepShapesSelectedCommand({selectedShape()}, {}, canvas()->selectedShapesProxy(), KisCommandUtils::FlipFlopCommand::State::INITIALIZING, parentCommand);
    KUndo2Command *cmd = new SvgTextChangeTransformsOnRange(shape, m_textCursor.getPos(), m_textCursor.getAnchor(), offset, SvgTextChangeTransformsOnRange::OffsetAll, true, parentCommand);
    new KoKeepShapesSelectedCommand({}, {selectedShape()}, canvas()->selectedShapesProxy(), KisCommandUtils::FlipFlopCommand::State::FINALIZING, parentCommand);
    parentCommand->setText(cmd->text());
    canvas()->addCommand(parentCommand);
}

void SvgTextTool::slotUpdateTypeSettingMode()
{
    m_textCursor.setTypeSettingModeActive(m_optionManager->typeSettingMode());
    KoSvgTextShape *shape = selectedShape();
    Q_FOREACH(QObject *obj, m_typeSettingMovementMapper->children()) {
        QAction *a = qobject_cast<QAction*>(obj);
        if (a && shape) {
            a->setEnabled(m_optionManager->typeSettingMode());
        }
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

    rect |= m_textOutlineHelper->decorationRect();

    return rect;
}

void SvgTextTool::paint(QPainter &gc, const KoViewConverter &converter)
{
    if (!isActivated()) return;

    if (m_interactionStrategy) {
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

    m_textOutlineHelper->setDecorationThickness(decorationThickness());
    m_textOutlineHelper->setHandleRadius(handleRadius());
    m_textOutlineHelper->paint(&gc, converter);
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
        m_textCursor.paintDecorations(gc, qApp->palette().color(QPalette::Highlight), decorationThickness(), handleRadius());
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
        if (m_textOutlineHelper->contourModeButtonHovered(event->point)) {
            m_textOutlineHelper->toggleTextContourMode(selectedShape);
            event->accept();
            KoToolManager::instance()->switchToolRequested("InteractionTool");
            return;
        }
        if (m_highlightItem == HighlightItem::TypeSettingHandle) {
            SvgTextCursor::TypeSettingModeHandle handle = m_textCursor.typeSettingHandleAtPos(handleGrabRect(event->point));
            if (handle != SvgTextCursor::NoHandle) {
                if (!m_textCursor.setDominantBaselineFromHandle(handle)) {
                    m_interactionStrategy.reset(new SvgTextTypeSettingStrategy(this, selectedShape, &m_textCursor, handleGrabRect(event->point), event->modifiers()));
                    m_textCursor.setDrawTypeSettingHandle(false);
                }
                event->accept();
                return;
            }
        } else if (m_highlightItem == HighlightItem::MoveBorder) {
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
    KoPathShape *hoveredFlowShape = dynamic_cast<KoPathShape *>(canvas()->shapeManager()->shapeAt(event->point));
    QString shapeType;
    QPainterPath hoverPath = KisToolUtils::shapeHoverInfoCrossLayer(canvas(), event->point, shapeType);
    bool crossLayerPossible = !hoverPath.isEmpty() && shapeType == KoSvgTextShape_SHAPEID;

    if (!selectedShape && !hoveredShape && !hoveredFlowShape && !crossLayerPossible) {
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
    } else if (hoveredFlowShape) {
        QPointF point = canvas()->snapGuide()->snap(event->point, event->modifiers());
        m_interactionStrategy.reset(new SvgCreateTextStrategy(this, point, hoveredFlowShape));
        m_dragging = DragMode::Create;
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
    m_textCursor.updateModifiers(event->modifiers());

    if (m_interactionStrategy) {
        m_interactionStrategy->handleMouseMove(event->point, event->modifiers());
        if (m_dragging == DragMode::Create) {
            SvgCreateTextStrategy *c = dynamic_cast<SvgCreateTextStrategy*>(m_interactionStrategy.get());
            if (c && c->draggingInlineSize() && !c->hasWrappingShape()) {
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

            SvgTextCursor::TypeSettingModeHandle handle = m_textCursor.typeSettingHandleAtPos(handleGrabRect(event->point));
            m_textCursor.setTypeSettingHandleHovered(handle);
            if (handle != SvgTextCursor::NoHandle) {
                cursor = m_textCursor.cursorTypeForTypeSetting();
                m_highlightItem = HighlightItem::TypeSettingHandle;
            }

            if (m_highlightItem == HighlightItem::None) {
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
        const KoPathShape *hoveredFlowShape = dynamic_cast<KoPathShape *>(canvas()->shapeManager()->shapeAt(event->point));
        QPainterPath hoverPath = KisToolUtils::shapeHoverInfoCrossLayer(canvas(), event->point, shapeType, &isHorizontal);
        if (selectedShape && selectedShape == hoveredShape && m_highlightItem == HighlightItem::None) {
            if (selectedShape->writingMode() == KoSvgText::HorizontalTB) {
                cursor = m_ibeam_horizontal;
            } else {
                cursor = m_ibeam_vertical;
            }
        } else if (hoveredShape) {
            if (!hoveredShape->textWrappingAreas().isEmpty()) {
                Q_FOREACH(QPainterPath path, hoveredShape->textWrappingAreas()) {
                    m_hoveredShapeHighlightRect.addPath(hoveredShape->absoluteTransformation().map(path));
                }
            } else {
                m_hoveredShapeHighlightRect.addRect(hoveredShape->boundingRect());
            }
            if (hoveredShape->writingMode() == KoSvgText::HorizontalTB) {
                cursor = m_ibeam_horizontal;
            } else {
                cursor = m_ibeam_vertical;
            }
        } else if (hoveredFlowShape) {
            m_hoveredShapeHighlightRect.addPath(hoveredFlowShape->absoluteTransformation().map(hoveredFlowShape->outline()));
            cursor = m_text_in_shape;
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
        m_textCursor.setDrawTypeSettingHandle(true);
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
    m_textCursor.updateModifiers(event->modifiers());
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

void SvgTextTool::addMappedAction(KisSignalMapper *mapper, const QString &actionName, const int value)
{
    QAction *a = action(actionName);
    if (a) {
        connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
        mapper->setMapping(a, value);
        m_textCursor.registerPropertyAction(a, actionName);
    }
}

