/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 * Copyright (C) 2014 Jarosław Staniek <staniek@kde.org>
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
#include "reportscene.h"
#include "reportsceneview.h"
#include "KoReportDesigner.h"
#include "KoReportDesignerItemBase.h"
#include "krutils.h"
#include "KoReportPluginInterface.h"
#include "KoReportPluginManager.h"
#include "KoReportDesignerItemRectBase.h"
#include "KoReportDesignerItemLine.h"

// qt
#include <QLabel>
#include <QDomDocument>
#include <QLayout>
#include <QGridLayout>
#include <QMouseEvent>
#include <QApplication>

#include <KoDpi.h>
#include <KoRuler.h>
#include <KoZoomHandler.h>
#include <KoIcon.h>

#include <klocalizedstring.h>
#include <kdebug.h>

//
// ReportSection method implementations
//

ReportSection::ReportSection(KoReportDesigner * rptdes)
        : QWidget(rptdes)
{
    m_sectionData = new KRSectionData(this);
    connect(m_sectionData->propertySet(), SIGNAL(propertyChanged(KoProperty::Set&,KoProperty::Property&)),
            this, SLOT(slotPropertyChanged(KoProperty::Set&,KoProperty::Property&)));
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
    m_sceneView = new ReportSceneView(rptdes, m_scene, this);
    m_sceneView->setObjectName("scene view");
    m_sceneView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_resizeBar = new ReportResizeBar(this);

    connect(m_resizeBar, SIGNAL(barDragged(int)), this, SLOT(slotResizeBarDragged(int)));
    connect(m_reportDesigner, SIGNAL(pagePropertyChanged(KoProperty::Set&)),
        this, SLOT(slotPageOptionsChanged(KoProperty::Set&)));
    connect(m_scene, SIGNAL(clicked()), this, (SLOT(slotSceneClicked())));
    connect(m_scene, SIGNAL(lostFocus()), m_title, SLOT(update()));
    connect(m_title, SIGNAL(clicked()), this, (SLOT(slotSceneClicked())));

    glayout->addWidget(m_title, 0, 0, 1, 2);
    glayout->addWidget(m_sectionRuler, 1, 0);
    glayout->addWidget(m_sceneView , 1, 1);
    glayout->addWidget(m_resizeBar, 2, 0, 1, 2);
    m_sectionRuler->setFixedWidth(m_sectionRuler->sizeHint().width());

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
    slotSceneClicked(); // switches property set to this section

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
    KRUtils::setAttribute(section, "svg:height", m_sectionData->m_height->value().toDouble());
    section.setAttribute("fo:background-color", m_sectionData->backgroundColor().name());

    // now get a list of all the QGraphicsItems on this scene and output them.
    QGraphicsItemList list = m_scene->items();
    for (QGraphicsItemList::iterator it = list.begin();
            it != list.end(); ++it) {
        KoReportDesignerItemBase::buildXML((*it), doc, section);
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
    //kDebug() << "Section Height: " << h;
    m_scene->setSceneRect(0, 0, m_scene->width(), h);
    slotResizeBarDragged(0);

    m_sectionData->m_backgroundColor->setValue(QColor(section.toElement().attribute("fo:background-color", "#ffffff")));

    for (int i = 0; i < nl.count(); ++i) {
        node = nl.item(i);
        n = node.nodeName();
        if (n.startsWith("report:")) {
            //Load objects
            //report:line is a special case as it is not a plugin
            QString reportItemName = n.mid(qstrlen("report:"));
            if (reportItemName == "line") {
                (new KoReportDesignerItemLine(node, m_sceneView->designer(), m_scene))->setVisible(true);
                continue;
            }
            KoReportPluginManager* manager = KoReportPluginManager::self();
            KoReportPluginInterface *plugin = manager->plugin(reportItemName);
            if (plugin) {
                QObject *obj = plugin->createDesignerInstance(node, m_reportDesigner, m_scene);
                if (obj) {
                    KoReportDesignerItemRectBase *entity = dynamic_cast<KoReportDesignerItemRectBase*>(obj);
                    if (entity) {
                        entity->setVisible(true);
                    }
                    continue;
                }
            }
        }
        kWarning() << "Encountered unknown node while parsing section: " << n;
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
    
    m_sectionData->m_height->setOption("unit", unit.symbol());

    //update items position with unit
    QList<QGraphicsItem*> itms = m_scene->items();
    for (int i = 0; i < itms.size(); ++i) {
        KoReportItemBase *obj = dynamic_cast<KoReportItemBase*>(itms[i]);
        if (obj) {
            obj->setUnit(unit);
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
    m_reportDesigner->changeSet(m_sectionData->propertySet());
}

void ReportSection::slotPropertyChanged(KoProperty::Set &s, KoProperty::Property &p)
{
    Q_UNUSED(s)
    //kDebug() << p.name();
    
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

void ReportSection::setSectionCursor(const QCursor& c)
{
    if (m_sceneView)
        m_sceneView->setCursor(c);
}

void ReportSection::unsetSectionCursor()
{
    if (m_sceneView)
        m_sceneView->unsetCursor();
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
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setMinimumHeight(qMax(fontMetrics().lineSpacing(), IconSize(KIconLoader::Small) + 2));
}

ReportSectionTitle::~ReportSectionTitle()
{
}

//! \return true if \a o has parent \a par.
static bool hasParent(QObject* par, QObject* o)
{
    if (!o || !par)
        return false;
    while (o && o != par)
        o = o->parent();
    return o == par;
}

static void replaceColors(QPixmap* original, const QColor& color)
{
    QImage dest(original->toImage());
    QPainter p(&dest);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(dest.rect(), color);
    *original = QPixmap::fromImage(dest);
}

void ReportSectionTitle::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    ReportSection* _section = dynamic_cast<ReportSection*>(parent());

    const bool current = _section->m_scene == _section->m_reportDesigner->activeScene();
    QPalette::ColorGroup cg = QPalette::Inactive;
    QWidget *activeWindow = QApplication::activeWindow();
    if (activeWindow) {
        QWidget *par = activeWindow->focusWidget();
        if (qobject_cast<ReportSceneView*>(par)) {
            par = par->parentWidget(); // we're close, pick common parent
        }
        if (hasParent(par, this)) {
            cg = QPalette::Active;
        }
    }
    if (current) {
        painter.fillRect(rect(), palette().brush(cg, QPalette::Highlight));
    }
    painter.setPen(palette().color(cg, current ? QPalette::HighlightedText : QPalette::WindowText));
    QPixmap pixmap(koSmallIcon("arrow-down"));
    replaceColors(&pixmap, painter.pen().color());
    const int left = 25;
    painter.drawPixmap(QPoint(left, (height() - pixmap.height()) / 2), pixmap);

    painter.drawText(rect().adjusted(left + pixmap.width() + 4, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, text());
    QFrame::paintEvent(event);
}

void ReportSectionTitle::mousePressEvent(QMouseEvent *event)
{
    QLabel::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
}
