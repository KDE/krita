/* This file is part of the KDE project

   Copyright 2017 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "SvgTextTool.h"
#include "KoSvgTextShape.h"
#include "SvgTextChangeCommand.h"

#include <QLabel>
#include <QToolButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QApplication>
#include <QGroupBox>
#include <QFontDatabase>
#include <QButtonGroup>

#include <klocalizedstring.h>

#include <KisPart.h>
#include <kis_canvas2.h>
#include <KSharedConfig>

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

#include "SvgTextEditor.h"

SvgTextTool::SvgTextTool(KoCanvasBase *canvas)
    : KoToolBase(canvas)
    , m_shape(0)
    , m_hoverShape(0)
    , m_editor(0)
    , m_dragStart( 0, 0)
    , m_dragEnd( 0, 0)
    , m_dragging(false)
{
}

SvgTextTool::~SvgTextTool()
{
}

void SvgTextTool::activate(ToolActivation activation, const QSet<KoShape *> &shapes)
{
    KoToolBase::activate(activation, shapes);

    foreach (KoShape *shape, shapes) {
        m_shape = dynamic_cast<KoSvgTextShape *>(shape);
        if (m_shape) {
            break;
        }
    }
    useCursor(Qt::ArrowCursor);
}

void SvgTextTool::deactivate()
{
    if (m_shape) {
        QRectF rect = m_shape->boundingRect();
        m_shape = 0;
        canvas()->updateCanvas(rect);
    }
    KoToolBase::deactivate();
}

QWidget *SvgTextTool::createOptionWidget()
{
    QWidget *optionWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(optionWidget);
    m_configGroup = KSharedConfig::openConfig()->group(toolId());

    QGroupBox *defsOptions = new QGroupBox(i18n("Create new texts with..."));
    QVBoxLayout *defOptionsLayout = new QVBoxLayout();
    defsOptions->setLayout(defOptionsLayout);
    m_defFont = new QFontComboBox();
    QString storedFont = m_configGroup.readEntry<QString>("defaultFont", QApplication::font().family());
    m_defFont->setCurrentFont(QFont(storedFont));
    defsOptions->layout()->addWidget(m_defFont);
    m_defPointSize = new QComboBox();
    Q_FOREACH (int size, QFontDatabase::standardSizes()) {
        m_defPointSize->addItem(QString::number(size)+" pt");
    }
    int storedSize = m_configGroup.readEntry<int>("defaultSize", QApplication::font().pointSize());
    m_defPointSize->setCurrentIndex(QFontDatabase::standardSizes().indexOf(storedSize));

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

    defOptionsLayout->addLayout(alignButtons);
    layout->addWidget(defsOptions);
    connect(m_defAlignment, SIGNAL(buttonClicked(int)), this, SLOT(storeDefaults()));
    connect(m_defFont, SIGNAL(currentFontChanged(QFont)), this, SLOT(storeDefaults()));
    connect(m_defPointSize, SIGNAL(currentIndexChanged(int)), this, SLOT(storeDefaults()));

    m_edit = new QPushButton(optionWidget);
    m_edit->setText(i18n("Edit Text"));
    connect(m_edit, SIGNAL(clicked(bool)), SLOT(showEditor()));
    layout->addWidget(m_edit);

    return optionWidget;
}

void SvgTextTool::showEditor()
{
    if (!m_shape) return;
    if (!m_editor) {
        m_editor = new SvgTextEditor(qApp->activeWindow(), Qt::Tool);
        m_editor->setWindowModality(Qt::WindowModal);
        connect(m_editor, SIGNAL(textUpdated(QString,QString)), SLOT(textUpdated(QString,QString)));
    }
    m_editor->setShape(m_shape);
    m_editor->show();
}

void SvgTextTool::textUpdated(const QString &svg, const QString &defs)
{
    SvgTextChangeCommand *cmd = new SvgTextChangeCommand(m_shape, svg, defs);
    canvas()->addCommand(cmd);
}

QString SvgTextTool::generateDefs()
{
    QString font = m_defFont->currentFont().family();
    QString size = QString::number(QFontDatabase::standardSizes().at(m_defPointSize->currentIndex()));
    QString textAnchor = "middle";
    if (m_defAlignment->button(0)->isChecked()) {
        textAnchor = "start";
    }
    if (m_defAlignment->button(2)->isChecked()) {
        textAnchor = "end";
    }

    return QString("<defs>\n <style>\n  text {\n   font-family:'%1';\n   font-size:%2 ;   text-anchor:%3;\n  }\n </style>\n</defs>").arg(font, size, textAnchor);
}

void SvgTextTool::storeDefaults()
{
    m_configGroup = KSharedConfig::openConfig()->group(toolId());
    m_configGroup.writeEntry("defaultFont", m_defFont->currentFont().family());
    m_configGroup.writeEntry("defaultSize", QFontDatabase::standardSizes().at(m_defPointSize->currentIndex()));
    m_configGroup.writeEntry("defaultAlignment", m_defAlignment->checkedId());
}

void SvgTextTool::paint(QPainter &gc, const KoViewConverter &converter)
{
    if (m_dragging) {
        gc.save();
        gc.setPen(Qt::black);
        QRectF rect = converter.documentToView(QRectF(m_dragStart, m_dragEnd));
        gc.drawRect(rect);
        gc.restore();
    }
    if (m_shape) {
        gc.save();
        gc.setPen(Qt::black);
        QRectF rect = converter.documentToView(m_shape->boundingRect());
        gc.drawRect(rect);
        QPen dotted = QPen(QColor(255, 255, 255, 200));
        dotted.setStyle(Qt::DotLine);
        gc.setPen(dotted);
        gc.drawRect(rect);
        gc.restore();
    }
    if (m_hoverShape) {
        gc.save();
        gc.setPen(Qt::cyan);
        QRectF rect = converter.documentToView(m_hoverShape->boundingRect());
        gc.drawRect(rect);
        gc.restore();
    }
}

void SvgTextTool::mousePressEvent(KoPointerEvent *event)
{
    if (canvas()->shapeManager()->shapeAt(event->point) != m_shape || !m_shape) {
        KoSvgTextShape *shape = dynamic_cast<KoSvgTextShape *>(canvas()->shapeManager()->shapeAt(event->point));

        if (shape) {
            canvas()->shapeManager()->selection()->deselectAll();
            canvas()->shapeManager()->selection()->select(shape);
            m_shape = shape;
        } else {
            m_dragStart = event->point;
            m_dragging = true;
        }

        event->accept();
    } else {
        event->ignore();
    }
}

void SvgTextTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        m_dragEnd = event->point;
        canvas()->updateCanvas(QRectF(m_dragStart, m_dragEnd).normalized().adjusted(-100, -100, 100, 100));
        event->accept();
    } else {
        m_hoverShape = dynamic_cast<KoSvgTextShape *>(canvas()->shapeManager()->shapeAt(event->point));
        if (m_hoverShape) {
            canvas()->updateCanvas(m_hoverShape->boundingRect().adjusted(-100, -100, 100, 100));
        } else {
            canvas()->updateCanvas(QRectF(event->point, event->point).adjusted(-100, -100, 100, 100));
        }
        event->ignore();
    }
}

void SvgTextTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value("KoSvgTextShapeID");
        KoProperties *params = new KoProperties();//Fill these with "svgText", "defs" and "shapeRect"
        params->setProperty("defs", QVariant(generateDefs()));
        if (m_dragging) {
            m_dragEnd = event->point;
            m_dragging = false;
            params->setProperty("shapeRect", QVariant(QRectF(m_dragStart, m_dragEnd).normalized()));
        }
        KoShape *textShape = factory->createShape( params, canvas()->shapeController()->resourceManager());


        KUndo2Command *cmd = canvas()->shapeController()->addShape(textShape, 0);
        canvas()->addCommand(cmd);
        canvas()->shapeManager()->selection()->deselectAll();
        canvas()->shapeManager()->selection()->select(textShape);
        m_shape = dynamic_cast<KoSvgTextShape *>(textShape);
        showEditor();
    }
    event->accept();
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
    if (canvas()->shapeManager()->shapeAt(event->point) != m_shape) {
        event->ignore(); // allow the event to be used by another
        return;
    }
    showEditor();
}

