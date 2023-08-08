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
#include "SvgInlineSizeHelper.h"
#include "SvgMoveTextCommand.h"
#include "SvgMoveTextStrategy.h"
#include "SvgTextChangeCommand.h"

#include <QLabel>
#include <QPainterPath>
#include <QToolButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QApplication>
#include <QGroupBox>
#include <QFontDatabase>
#include <QButtonGroup>
#include <QMenuBar>

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

#include "KisHandlePainterHelper.h"
#include <commands/KoKeepShapesSelectedCommand.h>

using SvgInlineSizeHelper::InlineSizeInfo;


SvgTextTool::SvgTextTool(KoCanvasBase *canvas)
    : KoToolBase(canvas)
{
}

SvgTextTool::~SvgTextTool()
{
    if(m_editor) {
        m_editor->close();
    }
    delete m_defAlignment;
}

void SvgTextTool::activate(const QSet<KoShape *> &shapes)
{
    KoToolBase::activate(shapes);
    useCursor(Qt::ArrowCursor);
    auto uploadColorToResourceManager = [this](KoShape *shape) {
        m_originalColor = canvas()->resourceManager()->foregroundColor();
        KoShapeFillWrapper wrapper(shape, KoFlake::Fill);
        KoColor color;
        color.fromQColor(wrapper.color());
        canvas()->resourceManager()->setForegroundColor(color);
    };

    if (shapes.size() == 1) {
        KoSvgTextShape *textShape = dynamic_cast<KoSvgTextShape*>(*shapes.constBegin());
        if (!textShape) {
            koSelection()->deselectAll();
        } else {
            uploadColorToResourceManager(textShape);
            // if we are a text shape...and the proxy tells us we want to edit the shape. open the text editor
            if (canvas()->selectedShapesProxy()->isRequestingToBeEdited()) {
                showEditor();
            }
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
            uploadColorToResourceManager(foundTextShape);
            koSelection()->select(foundTextShape);
        }
    }

    repaintDecorations();
}

void SvgTextTool::deactivate()
{
    KoToolBase::deactivate();
    if (m_originalColor) {
        canvas()->resourceManager()->setForegroundColor(*m_originalColor);
    }

    m_hoveredShapeHighlightRect = QPainterPath();

    repaintDecorations();
}

KisPopupWidgetInterface *SvgTextTool::popupWidget()
{
    return nullptr;
}

QWidget *SvgTextTool::createOptionWidget()
{
    QWidget *optionWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(optionWidget);
    m_configGroup = KSharedConfig::openConfig()->group(toolId());

    QGroupBox *defsOptions = new QGroupBox(i18n("Create new texts with..."));
    QVBoxLayout *defOptionsLayout = new QVBoxLayout(defsOptions);

    m_defFont = new QFontComboBox();
    QString storedFont = m_configGroup.readEntry<QString>("defaultFont", QApplication::font().family());
    m_defFont->setCurrentFont(QFont(storedFont));
    defOptionsLayout->addWidget(m_defFont);
    m_defPointSize = new QComboBox();
    Q_FOREACH (int size, QFontDatabase::standardSizes()) {
        m_defPointSize->addItem(QString::number(size)+" pt");
    }
    int storedSize = m_configGroup.readEntry<int>("defaultSize", QApplication::font().pointSize());
#ifdef Q_OS_ANDROID
    // HACK: on some devices where android.R.styleable exists, Qt's platform
    // plugin sets the pixelSize of a font, which returns -1 when asked for pointSize.
    //
    // The way to fetch font in Qt from SDK is deprecated in newer Android versions.
    if (storedSize <= 0) {
        storedSize = 18;  // being one of the standardSizes
    }
#endif
    int sizeIndex = 0;
    if (QFontDatabase::standardSizes().contains(storedSize)) {
        sizeIndex = QFontDatabase::standardSizes().indexOf(storedSize);
    }
    m_defPointSize->setCurrentIndex(sizeIndex);

    int checkedAlignment = m_configGroup.readEntry<int>("defaultAlignment", 0);

    m_defAlignment = new QButtonGroup();
    QHBoxLayout *alignButtons = new QHBoxLayout();
    alignButtons->addWidget(m_defPointSize);
    QToolButton *alignLeft = new QToolButton();
    alignLeft->setIcon(KisIconUtils::loadIcon("format-justify-left"));
    alignLeft->setCheckable(true);
    alignLeft->setAutoRaise(true);

    alignLeft->setToolTip(i18n("Anchor text to the left."));
    m_defAlignment->addButton(alignLeft, 0);
    alignButtons->addWidget(alignLeft);

    QToolButton *alignCenter = new QToolButton();
    alignCenter->setIcon(KisIconUtils::loadIcon("format-justify-center"));
    alignCenter->setCheckable(true);
    alignCenter->setAutoRaise(true);
    m_defAlignment->addButton(alignCenter, 1);
    alignCenter->setToolTip(i18n("Anchor text to the middle."));

    alignButtons->addWidget(alignCenter);

    QToolButton *alignRight = new QToolButton();
    alignRight->setIcon(KisIconUtils::loadIcon("format-justify-right"));
    alignRight->setCheckable(true);
    alignRight->setAutoRaise(true);
    m_defAlignment->addButton(alignRight, 2);
    alignRight->setToolTip(i18n("Anchor text to the right."));
    alignButtons->addWidget(alignRight);

    m_defAlignment->setExclusive(true);
    if (checkedAlignment<1) {
        alignLeft->setChecked(true);
    } else if (checkedAlignment==1) {
        alignCenter->setChecked(true);
    } else if (checkedAlignment==2) {
        alignRight->setChecked(true);
    } else {
        alignLeft->setChecked(true);
    }

    double storedLetterSpacing = m_configGroup.readEntry<double>("defaultLetterSpacing", 0.0);
    m_defLetterSpacing = new QDoubleSpinBox();
    m_defLetterSpacing->setToolTip(i18n("Letter Spacing"));
    m_defLetterSpacing->setRange(-20.0, 20.0);
    m_defLetterSpacing->setSingleStep(0.5);
    m_defLetterSpacing->setValue(storedLetterSpacing);
    alignButtons->addWidget(m_defLetterSpacing);

    defOptionsLayout->addLayout(alignButtons);
    layout->addWidget(defsOptions);
    connect(m_defAlignment, SIGNAL(buttonClicked(int)), this, SLOT(storeDefaults()));
    connect(m_defFont, SIGNAL(currentFontChanged(QFont)), this, SLOT(storeDefaults()));
    connect(m_defPointSize, SIGNAL(currentIndexChanged(int)), this, SLOT(storeDefaults()));
    connect(m_defLetterSpacing, SIGNAL(valueChanged(double)), SLOT(storeDefaults()));

    m_edit = new QPushButton(optionWidget);
    m_edit->setText(i18n("Edit Text"));
    connect(m_edit, SIGNAL(clicked(bool)), SLOT(showEditor()));
    layout->addWidget(m_edit);

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

    KIS_SAFE_ASSERT_RECOVER_NOOP(shapes.size() == 1);
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

        connect(m_editor, SIGNAL(textUpdated(KoSvgTextShape*,QString,QString,bool)), SLOT(textUpdated(KoSvgTextShape*,QString,QString,bool)));
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

void SvgTextTool::textUpdated(KoSvgTextShape *shape, const QString &svg, const QString &defs, bool richTextUpdated)
{
    SvgTextChangeCommand *cmd = new SvgTextChangeCommand(shape, svg, defs, richTextUpdated);
    canvas()->addCommand(cmd);
}

void SvgTextTool::slotTextEditorClosed()
{
    // change tools to the shape selection tool when we close the text editor to allow moving and further editing of the object.
    // most of the time when we edit text, the shape selection tool is where we left off anyway
    KoToolManager::instance()->switchToolRequested("InteractionTool");
}

QString SvgTextTool::generateDefs(const QString &extraProperties)
{
    QString font = m_defFont->currentFont().family();
    QString size = QString::number(QFontDatabase::standardSizes().at(m_defPointSize->currentIndex() > -1 ? m_defPointSize->currentIndex() : 0));

    QString textAnchor = "middle";
    if (m_defAlignment->button(0)->isChecked()) {
        textAnchor = "start";
    }
    if (m_defAlignment->button(2)->isChecked()) {
        textAnchor = "end";
    }

    QString fontColor = canvas()->resourceManager()->foregroundColor().toQColor().name();
    QString letterSpacing = QString::number(m_defLetterSpacing->value());

    return QString("<defs>\n <style>\n  text {\n   font-family:'%1';\n   font-size:%2 ; fill:%3 ;  text-anchor:%4; letter-spacing:%5;%6\n  }\n </style>\n</defs>").arg(font, size, fontColor, textAnchor, letterSpacing, extraProperties);
}

void SvgTextTool::storeDefaults()
{
    m_configGroup = KSharedConfig::openConfig()->group(toolId());
    m_configGroup.writeEntry("defaultFont", m_defFont->currentFont().family());
    m_configGroup.writeEntry("defaultSize", QFontDatabase::standardSizes().at(m_defPointSize->currentIndex() > -1 ? m_defPointSize->currentIndex() : 0));
    m_configGroup.writeEntry("defaultAlignment", m_defAlignment->checkedId());
    m_configGroup.writeEntry("defaultLetterSpacing", m_defLetterSpacing->value());
}

QFont SvgTextTool::defaultFont() const
{
    int size = QFontDatabase::standardSizes().at(m_defPointSize->currentIndex() > -1 ? m_defPointSize->currentIndex() : 0);
    QFont font = m_defFont->currentFont();
    font.setPointSize(size);
    return font;
}

Qt::Alignment SvgTextTool::horizontalAlign() const
{
    if (m_defAlignment->button(1)->isChecked()) {
        return Qt::AlignHCenter;
    }
    if (m_defAlignment->button(2)->isChecked()) {
        return Qt::AlignRight;
    }
    return Qt::AlignLeft;
}

QRectF SvgTextTool::decorationsRect() const
{
    QRectF rect;
    KoSvgTextShape *const shape = selectedShape();
    if (shape) {
        rect |= shape->boundingRect();

        const QPointF anchor = shape->absoluteTransformation().map(QPointF());
        rect |= kisGrowRect(QRectF(anchor, anchor), handleRadius());

        if (std::optional<InlineSizeInfo> info = InlineSizeInfo::fromShape(shape)) {
            rect |= info->boundingRect();
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

    gc.setTransform(converter.documentToView(), true);

    KisHandlePainterHelper handlePainter(&gc);

    KoSvgTextShape *shape = selectedShape();
    if (shape) {
        if (m_dragging != DragMode::InlineSizeHandle) {
            handlePainter.setHandleStyle(KisHandleStyle::primarySelection());
            QPainterPath path;
            path.addRect(shape->boundingRect());
            handlePainter.drawPath(path);
            if (m_highlightItem == HighlightItem::MoveBorder) {
                handlePainter.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());
            }
            handlePainter.drawHandleCircle(shape->absoluteTransformation().map(QPointF()), KoToolBase::handleRadius());
        }

        if (std::optional<InlineSizeInfo> info = InlineSizeInfo::fromShape(shape)) {
            handlePainter.setHandleStyle(KisHandleStyle::secondarySelection());
            handlePainter.drawConnectionLine(info->baselineLine());
            handlePainter.drawConnectionLine(info->nonEditLine());
            if (m_highlightItem == HighlightItem::InlineSizeHandle) {
                handlePainter.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());
            }
            handlePainter.drawConnectionLine(info->editLine());
        }
    }

    if (!m_hoveredShapeHighlightRect.isEmpty()) {
        handlePainter.setHandleStyle(KisHandleStyle::highlightedPrimaryHandlesWithSolidOutline());
        QPainterPath path;
        path.addPath(m_hoveredShapeHighlightRect);
        handlePainter.drawPath(path);
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
        } else if (m_highlightItem == HighlightItem::InlineSizeHandle) {
            m_interactionStrategy.reset(new SvgInlineSizeChangeStrategy(this, selectedShape, event->point));
            m_dragging = DragMode::InlineSizeHandle;
            event->accept();
            return;
        }
    }

    KoSvgTextShape *hoveredShape = dynamic_cast<KoSvgTextShape *>(canvas()->shapeManager()->shapeAt(event->point));

    if (!selectedShape || hoveredShape != selectedShape) {
        canvas()->shapeManager()->selection()->deselectAll();

        if (hoveredShape) {
            canvas()->shapeManager()->selection()->select(hoveredShape);
        } else {
            m_interactionStrategy.reset(new SvgCreateTextStrategy(this, event->point));
            m_dragging = DragMode::Create;
            event->accept();
        }
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

    if (m_interactionStrategy) {
        m_interactionStrategy->handleMouseMove(event->point, event->modifiers());
        event->accept();
    } else {
        m_highlightItem = HighlightItem::None;
        KoSvgTextShape *const selectedShape = this->selectedShape();
        Qt::CursorShape cursor = Qt::ArrowCursor;
        if (selectedShape) {
            const qreal sensitivity = grabSensitivityInPt();

            if (std::optional<InlineSizeInfo> info = InlineSizeInfo::fromShape(selectedShape)) {
                const QPolygonF zone = info->editLineGrabRect(sensitivity);
                if (zone.containsPoint(event->point, Qt::OddEvenFill)) {
                    m_highlightItem = HighlightItem::InlineSizeHandle;
                    cursor = lineToCursor(info->baselineLine(), canvas());
                }
            }

            if (m_highlightItem == HighlightItem::None) {
                const QPolygonF textOutline = selectedShape->absoluteTransformation().map(selectedShape->outlineRect());
                const QRectF moveBorderRegion = kisGrowRect(selectedShape->boundingRect(), sensitivity);
                if (moveBorderRegion.contains(event->point) && !textOutline.containsPoint(event->point, Qt::OddEvenFill)) {
                    m_highlightItem = HighlightItem::MoveBorder;
                    cursor = Qt::SizeAllCursor;
                }
            }
        }
        useCursor(cursor);

        KoSvgTextShape *hoveredShape = dynamic_cast<KoSvgTextShape *>(canvas()->shapeManager()->shapeAt(event->point));
        if (hoveredShape && m_highlightItem == HighlightItem::None) {
            m_hoveredShapeHighlightRect = {};
            if (hoveredShape->shapesInside().isEmpty()) {
                m_hoveredShapeHighlightRect.addRect(hoveredShape->boundingRect());
            } else {
                Q_FOREACH(KoShape *shape, hoveredShape->shapesInside()) {
                    KoPathShape *path = dynamic_cast<KoPathShape *>(shape);
                    if (path) {
                        m_hoveredShapeHighlightRect.addPath(hoveredShape->transformation().map(path->transformation().map(path->outline())));
                    }
                }
            }
        } else {
            m_hoveredShapeHighlightRect = QPainterPath();
        }
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
            canvas()->addCommand(command);
        }
        m_interactionStrategy = nullptr;
        m_dragging = DragMode::None;
        event->accept();
    } else if (m_editor) {
        showEditor();
        event->accept();
    }
}

void SvgTextTool::keyPressEvent(QKeyEvent *event)
{
    if (m_interactionStrategy
        && (event->key() == Qt::Key_Control || event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift
            || event->key() == Qt::Key_Meta)) {
        m_interactionStrategy->handleMouseMove(m_lastMousePos, event->modifiers());
        event->accept();
        return;
    }

    if (m_interactionStrategy) {
        if (event->key() == Qt::Key_Escape) {
            m_dragging = DragMode::None;
            m_interactionStrategy->cancelInteraction();
            m_interactionStrategy = nullptr;
            useCursor(Qt::ArrowCursor);
            event->accept();
        } else {
            event->ignore();
        }
    } else if (event->key()==Qt::Key_Enter || event->key()==Qt::Key_Return) {
        showEditor();
        event->accept();
    } else {
        event->ignore();
    }
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

void SvgTextTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    if (canvas()->shapeManager()->shapeAt(event->point) != selectedShape()) {
        event->ignore(); // allow the event to be used by another
        return;
    }
    const QRectF updateRect = std::exchange(m_hoveredShapeHighlightRect, QPainterPath()).boundingRect();
    canvas()->updateCanvas(kisGrowRect(updateRect, 100));
    showEditor();
    if(m_editor) {
        m_editor->raise();
        m_editor->activateWindow();
    }
    event->accept();
}

qreal SvgTextTool::grabSensitivityInPt() const
{
    const int sensitivity = grabSensitivity();
    return canvas()->viewConverter()->viewToDocumentX(sensitivity);
}

