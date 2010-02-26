/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "reportsection.h"
#include "KoReportDesigner.h"
#include "reportentities.h"
#include "reportentitylabel.h"
#include "reportentityfield.h"
#include "reportentitytext.h"
#include "reportentityline.h"
#include "reportentitybarcode.h"
#include "reportentityimage.h"
#include "reportentitychart.h"
#include "reportentitycheck.h"
#include "reportentityshape.h"

#include "reportscene.h"
#include "reportsceneview.h"

// qt
#include <qlabel.h>
#include <qdom.h>
#include <qlayout.h>
#include <QGridLayout>
#include <QMouseEvent>

#include <KoDpi.h>
#include <KoRuler.h>
#include <KoZoomHandler.h>
#include <koproperty/EditorView.h>
#include <KColorScheme>
#include <QBitmap>

#include <kdebug.h>

static const char *arrow_xpm[] = {
    /* width height num_colors chars_per_pixel */
    "    11    12       2            1",
    /* colors */
    ". c None",
    "# c #555555",
    /*   data   */
    "...........",
    "...#####...",
    "...#####...",
    "...#####...",
    "...#####...",
    "...#####...",
    "###########",
    ".#########.",
    "..#######.-",
    "...#####...",
    "....###....",
    ".....#....."
};

//
// ReportSection method implementations
//

ReportSection::ReportSection(KoReportDesigner * rptdes, const char * name)
        : QWidget(rptdes)
{
    Q_UNUSED(name)

    m_sectionData = new KRSectionData();
    connect(m_sectionData->properties(), SIGNAL(propertyChanged(KoProperty::Set&, KoProperty::Property&)),
            this, SLOT(slotPropertyChanged(KoProperty::Set&, KoProperty::Property&)));
    int dpiY = KoDpi::dpiY();

    m_reportDesigner = rptdes;
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QGridLayout * glayout = new QGridLayout(this);
    glayout->setSpacing(0);
    glayout->setMargin(0);
    glayout->setColumnStretch(1, 1);
    glayout->setRowStretch(1, 1);
    glayout->setSizeConstraint(QLayout::SetFixedSize);

    // ok create the base interface
    m_title = new ReportSectionTitle(this);
    m_title->setObjectName("detail");
    m_title->setText(i18n("Detail"));

    m_sectionRuler = new KoRuler(this, Qt::Vertical, m_reportDesigner->zoomHandler());
    m_sectionRuler->setUnit(m_reportDesigner->pageUnit());
    m_scene = new ReportScene(m_reportDesigner->pageWidthPx(), dpiY, rptdes);
    m_sceneView = new ReportSceneView(rptdes, m_scene, this, "scene view");
    m_sceneView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_resizeBar = new ReportResizeBar(this);

    connect(m_resizeBar, SIGNAL(barDragged(int)), this, SLOT(slotResizeBarDragged(int)));
    connect(m_reportDesigner, SIGNAL(pagePropertyChanged(KoProperty::Set&)),
        this, SLOT(slotPageOptionsChanged(KoProperty::Set&)));
    connect(m_scene, SIGNAL(clicked()), this, (SLOT(slotSceneClicked())));

    glayout->addWidget(m_title, 0, 0, 1, 2);
    glayout->addWidget(m_sectionRuler, 1, 0);
    glayout->addWidget(m_sceneView , 1, 1);
    glayout->addWidget(m_resizeBar, 2, 0, 1, 2);

    setLayout(glayout);
    slotResizeBarDragged(0);
}

ReportSection::~ReportSection()
{
    // Qt should be handling everything for us
}

void ReportSection::setTitle(const QString & s)
{
    m_title->setText(s);
}

void ReportSection::slotResizeBarDragged(int delta)
{
    if (m_sceneView->designer() && m_sceneView->designer()->propertySet()->property("page-size").value().toString() == "Labels") {
        return; // we don't want to allow this on reports that are for labels
    }

    qreal h = m_scene->height() + delta;

    if (h < 1) h = 1;

    h = m_scene->gridPoint(QPointF(0, h)).y();
    m_sectionData->m_height->setValue(INCH_TO_POINT(h/KoDpi::dpiY()));
    m_sectionRuler->setRulerLength(h);

    m_scene->setSceneRect(0, 0, m_scene->width(), h);
    m_sceneView->resizeContents(QSize(m_scene->width(), h));

    m_reportDesigner->setModified(true);
}

void ReportSection::buildXML(QDomDocument &doc, QDomElement &section)
{
    QString un = m_sectionData->m_height->option("unit", "cm").toString();

    section.setAttribute("svg:height", KoUnit::unit(un).toUserStringValue(m_sectionData->m_height->value().toDouble()) + un);
    section.setAttribute("fo:background-color", m_sectionData->backgroundColor().name());

    // now get a list of all the QGraphicsItems on this scene and output them.
    QGraphicsItemList list = m_scene->items();
    for (QGraphicsItemList::iterator it = list.begin();
            it != list.end(); ++it) {
        ReportEntity::buildXML((*it), doc, section);
    }
}

void ReportSection::initFromXML(QDomNode & section)
{
    QDomNodeList nl = section.childNodes();
    QDomNode node;
    QString n;

    qreal h = KoUnit::parseValue(section.toElement().attribute("svg:height", "2.0cm"));
    m_sectionData->m_height->setValue(h);
    
    h  = POINT_TO_INCH(h) * KoDpi::dpiY();
    kDebug() << "Section Height: " << h;
    m_scene->setSceneRect(0, 0, m_scene->width(), h);
    slotResizeBarDragged(0);

    m_sectionData->m_backgroundColor->setValue(QColor(section.toElement().attribute("fo:background-color", "#ffffff")));

    for (int i = 0; i < nl.count(); ++i) {
        node = nl.item(i);
        n = node.nodeName();

        //Objects
        if (n == "report:label") {
            (new ReportEntityLabel(node, m_sceneView->designer(), m_scene))->setVisible(true);
        } else if (n == "report:field") {
            (new ReportEntityField(node, m_sceneView->designer(), m_scene))->setVisible(true);
        } else if (n == "report:text") {
            (new ReportEntityText(node, m_sceneView->designer(), m_scene))->setVisible(true);
        } else if (n == "report:line") {
            (new ReportEntityLine(node, m_sceneView->designer(), m_scene))->setVisible(true);
        } else if (n == "report:barcode") {
            (new ReportEntityBarcode(node, m_sceneView->designer(), m_scene))->setVisible(true);
        } else if (n == "report:image") {
            (new ReportEntityImage(node, m_sceneView->designer(), m_scene))->setVisible(true);
        } else if (n == "report:chart") {
            (new ReportEntityChart(node, m_sceneView->designer(), m_scene))->setVisible(true);
        } else if (n == "report:check") {
            (new ReportEntityCheck(node, m_sceneView->designer(), m_scene))->setVisible(true);
        } else if (n == "report:shape") {
            (new ReportEntityShape(node, m_sceneView->designer(), m_scene))->setVisible(true);
        } else {
            kDebug() << "Encountered unknown node while parsing section: " << n;
        }
    }
}

QSize ReportSection::sizeHint() const
{
    return QSize(m_scene->width()  + m_sectionRuler->frameSize().width(), m_title->frameSize().height() + m_sceneView->sizeHint().height() + m_resizeBar->frameSize().height());
}

void ReportSection::slotPageOptionsChanged(KoProperty::Set &set)
{
    Q_UNUSED(set)

    KoUnit unit = m_reportDesigner->pageUnit();
    
    m_sectionData->m_height->setOption("unit", KoUnit::unitName(unit));

    //update items position with unit
    QList<QGraphicsItem*> itms = m_scene->items();
    for (int i = 0; i < itms.size(); ++i) {
	int typ = dynamic_cast<KRObjectData*>(itms[i])->type();
	
        if ( typ >= KRObjectData::EntityLabel && typ < KRObjectData::EntityLast) {
            dynamic_cast<ReportRectEntity*>(itms[i])->setUnit(unit);
        }
        if (typ == KRObjectData::EntityLine) {
            dynamic_cast<ReportEntityLine*>(itms[i])->setUnit(unit);
        }
    }

    m_scene->setSceneRect(0, 0, m_reportDesigner->pageWidthPx(), m_scene->height());
    m_title->setMinimumWidth(m_reportDesigner->pageWidthPx() + m_sectionRuler->frameSize().width());
    m_sectionRuler->setUnit(m_reportDesigner->pageUnit());

    //Trigger a redraw of the background
    m_sceneView->resetCachedContent();

    m_reportDesigner->adjustSize();
    m_reportDesigner->repaint();
    
    slotResizeBarDragged(0);
}

void ReportSection::slotSceneClicked()
{
    m_reportDesigner->setActiveScene(m_scene);
    m_reportDesigner->changeSet(m_sectionData->properties());
}

void ReportSection::slotPropertyChanged(KoProperty::Set &s, KoProperty::Property &p)
{
    Q_UNUSED(s)

    kDebug() << p.name();
    
    //Handle Background Color
    if (p.name() == "background-color") {
        m_scene->setBackgroundBrush(p.value().value<QColor>());
    }
    
    if (p.name() == "height") {
	m_scene->setSceneRect(0, 0, m_scene->width(), POINT_TO_INCH(p.value().toDouble()) * KoDpi::dpiY());
	slotResizeBarDragged(0);
    }

    if (m_reportDesigner)
        m_reportDesigner->setModified(true);

    m_sceneView->resetCachedContent();
    m_scene->update();
}

//
// class ReportResizeBar
//
ReportResizeBar::ReportResizeBar(QWidget * parent, Qt::WFlags f)
        : QFrame(parent, f)
{
    setCursor(QCursor(Qt::SizeVerCursor));
    setFrameStyle(QFrame::HLine);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

void ReportResizeBar::mouseMoveEvent(QMouseEvent * e)
{
    e->accept();
    emit barDragged(e->y());
}

//=============================================================================

ReportSectionTitle::ReportSectionTitle(QWidget*parent) : QLabel(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setMinimumHeight(20);
}

ReportSectionTitle::~ReportSectionTitle()
{

}

void ReportSectionTitle::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    KColorScheme colorScheme(QPalette::Active);

    QLinearGradient linearGrad(QPointF(0, 0), QPointF(width(), 0));

    ReportSection* _section = dynamic_cast<ReportSection*>(parent());

    if (_section->m_scene == _section->m_reportDesigner->activeScene()) {
        linearGrad.setColorAt(0, colorScheme.decoration(KColorScheme::HoverColor).color());
        linearGrad.setColorAt(1, colorScheme.decoration(KColorScheme::FocusColor).color());
    } else {
        linearGrad.setColorAt(0, colorScheme.background(KColorScheme::NormalBackground).color());
        linearGrad.setColorAt(1, colorScheme.foreground(KColorScheme::InactiveText).color());
    }

    painter.fillRect(rect(), linearGrad);

    painter.setPen(Qt::black);
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.drawPixmap(QPoint(25, (height() - 12) / 2), QPixmap(arrow_xpm));

    painter.drawText(rect().adjusted(40, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, text());
    QFrame::paintEvent(event);
}
