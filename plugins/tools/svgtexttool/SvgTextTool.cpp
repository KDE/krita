/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SvgTextTool.h"
#include "KoSvgTextShape.h"
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

#include <KoFileDialog.h>
#include <KoIcon.h>
#include <KoCanvasBase.h>
#include <KoImageCollection.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoShapeController.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactoryBase.h>
#include <KoPointerEvent.h>
#include <KoProperties.h>
#include <KoSelectedShapesProxy.h>
#include "KoToolManager.h"
#include "KoCanvasResourceProvider.h"

#include "SvgTextEditor.h"
#include "KisHandlePainterHelper.h"
#include <commands/KoKeepShapesSelectedCommand.h>


SvgTextTool::SvgTextTool(KoCanvasBase *canvas)
    : KoToolBase(canvas)
    , m_editor(0)
    , m_dragStart( 0, 0)
    , m_dragEnd( 0, 0)
    , m_dragging(false)
{
}

SvgTextTool::~SvgTextTool()
{
    if(m_editor) {
        m_editor->close();
    }
}

void SvgTextTool::activate(const QSet<KoShape *> &shapes)
{
    KoToolBase::activate(shapes);
    useCursor(Qt::ArrowCursor);

    if (shapes.size() == 1) {
        KoSvgTextShape *textShape = dynamic_cast<KoSvgTextShape*>(*shapes.constBegin());
        if (!textShape) {
            koSelection()->deselectAll();
        } else {
            // if we are a text shape...and the proxy tells us we want to edit the shape. open the text editor
            if (canvas()->selectedShapesProxy()->isRequestingToBeEdited()) {
                showEditor();
            }
        }
    } else if (shapes.size() > 1) {
        KoSvgTextShape *foundTextShape = 0;

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
    }
}

void SvgTextTool::deactivate()
{
    KoToolBase::deactivate();

    QRectF updateRect = m_hoveredShapeHighlightRect;

    KoSvgTextShape *shape = selectedShape();
    if (shape) {
        updateRect |= shape->boundingRect();
    }
    m_hoveredShapeHighlightRect = QRectF();

    canvas()->updateCanvas(updateRect);
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

    alignLeft->setToolTip(i18n("Anchor text to the left."));
    m_defAlignment->addButton(alignLeft, 0);
    alignButtons->addWidget(alignLeft);

    QToolButton *alignCenter = new QToolButton();
    alignCenter->setIcon(KisIconUtils::loadIcon("format-justify-center"));
    alignCenter->setCheckable(true);
    m_defAlignment->addButton(alignCenter, 1);
    alignCenter->setToolTip(i18n("Anchor text to the middle."));

    alignButtons->addWidget(alignCenter);

    QToolButton *alignRight = new QToolButton();
    alignRight->setIcon(KisIconUtils::loadIcon("format-justify-right"));
    alignRight->setCheckable(true);
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
        m_editor->setWindowFlags(Qt::Tool);
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

QString SvgTextTool::generateDefs()
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

    return QString("<defs>\n <style>\n  text {\n   font-family:'%1';\n   font-size:%2 ; fill:%3 ;  text-anchor:%4; letter-spacing:%5;\n  }\n </style>\n</defs>").arg(font, size, fontColor, textAnchor, letterSpacing);
}

void SvgTextTool::storeDefaults()
{
    m_configGroup = KSharedConfig::openConfig()->group(toolId());
    m_configGroup.writeEntry("defaultFont", m_defFont->currentFont().family());
    m_configGroup.writeEntry("defaultSize", QFontDatabase::standardSizes().at(m_defPointSize->currentIndex() > -1 ? m_defPointSize->currentIndex() : 0));
    m_configGroup.writeEntry("defaultAlignment", m_defAlignment->checkedId());
    m_configGroup.writeEntry("defaultLetterSpacing", m_defLetterSpacing->value());
}

void SvgTextTool::paint(QPainter &gc, const KoViewConverter &converter)
{
    if (!isActivated()) return;

    gc.setTransform(converter.documentToView(), true);

    KisHandlePainterHelper handlePainter(&gc);

    if (m_dragging) {
        QPolygonF poly(QRectF(m_dragStart, m_dragEnd));
        handlePainter.setHandleStyle(KisHandleStyle::primarySelection());
        handlePainter.drawRubberLine(poly);
    }

    KoSvgTextShape *shape = selectedShape();
    if (shape) {
        handlePainter.setHandleStyle(KisHandleStyle::primarySelection());
        QPainterPath path;
        path.addRect(shape->boundingRect());
        handlePainter.drawPath(path);
    }

    if (!m_hoveredShapeHighlightRect.isEmpty()) {
        handlePainter.setHandleStyle(KisHandleStyle::highlightedPrimaryHandlesWithSolidOutline());
        QPainterPath path;
        path.addRect(m_hoveredShapeHighlightRect);
        handlePainter.drawPath(path);
    }
}

void SvgTextTool::mousePressEvent(KoPointerEvent *event)
{
    KoSvgTextShape *selectedShape = this->selectedShape();
    KoSvgTextShape *hoveredShape = dynamic_cast<KoSvgTextShape *>(canvas()->shapeManager()->shapeAt(event->point));

    if (!selectedShape || hoveredShape != selectedShape) {
        canvas()->shapeManager()->selection()->deselectAll();

        if (hoveredShape) {
            canvas()->shapeManager()->selection()->select(hoveredShape);
        } else {
            m_dragStart = m_dragEnd = event->point;
            m_dragging = true;
            event->accept();
        }
    }
}

void SvgTextTool::mouseMoveEvent(KoPointerEvent *event)
{
    QRectF updateRect = m_hoveredShapeHighlightRect;

    if (m_dragging) {
        m_dragEnd = event->point;
        m_hoveredShapeHighlightRect = QRectF();
        updateRect |= QRectF(m_dragStart, m_dragEnd).normalized().toAlignedRect();
        event->accept();
    } else {
        KoSvgTextShape *hoveredShape = dynamic_cast<KoSvgTextShape *>(canvas()->shapeManager()->shapeAt(event->point));
        if (hoveredShape) {
            m_hoveredShapeHighlightRect = hoveredShape->boundingRect();
            updateRect |= m_hoveredShapeHighlightRect;
        } else {
            m_hoveredShapeHighlightRect = QRect();
        }
        event->ignore();
    }

    if (!updateRect.isEmpty()) {
        canvas()->updateCanvas(kisGrowRect(updateRect, 100));
    }
}

void SvgTextTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        QRectF rectangle = QRectF(m_dragStart, m_dragEnd).normalized();
        if (rectangle.width() < 4 && rectangle.height() < 4) {
            m_dragging = false;
            event->accept();
            return;
        }
        KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value("KoSvgTextShapeID");
        KoProperties *params = new KoProperties();//Fill these with "svgText", "defs" and "shapeRect"
        params->setProperty("defs", QVariant(generateDefs()));
        if (m_dragging) {
            m_dragEnd = event->point;
            m_dragging = false;

            //The following show only happen when we're creating preformatted text. If we're making
            //Word-wrapped text, it should take the rectangle unmodified.
            int size = QFontDatabase::standardSizes().at(m_defPointSize->currentIndex() > -1 ? m_defPointSize->currentIndex() : 0);
            QFont font = m_defFont->currentFont();
            font.setPointSize(size);
            rectangle.setTop(rectangle.top()+QFontMetrics(font).lineSpacing());
            if (m_defAlignment->button(1)->isChecked()) {
                rectangle.setLeft(rectangle.center().x());
            } else if (m_defAlignment->button(2)->isChecked()) {
                qreal right = rectangle.right();
                rectangle.setRight(right+10);
                rectangle.setLeft(right);
            }

            params->setProperty("shapeRect", QVariant(rectangle));
        }
        KoShape *textShape = factory->createShape( params, canvas()->shapeController()->resourceManager());

        KUndo2Command *parentCommand = new KUndo2Command();

        new KoKeepShapesSelectedCommand(koSelection()->selectedShapes(), {}, canvas()->selectedShapesProxy(), false, parentCommand);

        KUndo2Command *cmd = canvas()->shapeController()->addShape(textShape, 0, parentCommand);
        parentCommand->setText(cmd->text());

        new KoKeepShapesSelectedCommand({}, {textShape}, canvas()->selectedShapesProxy(), true, parentCommand);

        canvas()->addCommand(parentCommand);

        showEditor();
        event->accept();

    } else if (m_editor) {
        showEditor();
        event->accept();
    }
}

void SvgTextTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key()==Qt::Key_Enter || event->key()==Qt::Key_Return) {
        showEditor();
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
    showEditor();
    if(m_editor) {
        m_editor->raise();
        m_editor->activateWindow();
    }
    event->accept();
}

