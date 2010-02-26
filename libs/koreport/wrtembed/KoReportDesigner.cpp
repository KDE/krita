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

#include "KoReportDesigner.h"
#include "reportsection.h"
#include "reportscene.h"
#include "reportsceneview.h"
#include "reportentitylabel.h"
#include "reportentityfield.h"
#include "reportentitytext.h"
#include "reportentityline.h"
#include "reportentitybarcode.h"
#include "reportentityimage.h"
#include "reportentitychart.h"
#include "reportentityshape.h"
#include "reportentitycheck.h"
#include "reportsectiondetailgroup.h"
#include "reportpropertiesbutton.h"
#include "sectioneditor.h"
#include "reportsectiondetail.h"

#include <QLayout>
#include <qdom.h>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QMessageBox>

#include <koproperty/EditorView.h>
#include <KoRuler.h>
#include <KoZoomHandler.h>
#include <KoDpi.h>
#include <KoPageFormat.h>
#include <kaction.h>
#include <KLocale>
#include <KDebug>
#include <kross/core/manager.h>

//! Also add public method for runtime?
const char* ns = "http://kexi-project.org/report/2.0";

static QDomElement propertyToElement(QDomDocument* d, KoProperty::Property* p)
{
    QDomElement e = d->createElement("report:" + p->name().toLower());
    e.appendChild(d->createTextNode(p->value().toString()));
    return e;
}

//
// define and implement the ReportWriterSectionData class
// a simple class to hold/hide data in the ReportHandler class
//
class ReportWriterSectionData
{
public:
    ReportWriterSectionData() {
        selected_items_rw = 0;
        mouseAction = ReportWriterSectionData::MA_None;
        insertItem = KRObjectData::EntityNone;
    }
    virtual ~ReportWriterSectionData() {
        selected_items_rw = 0;
    }

    enum MouseAction {
        MA_None = 0,
        MA_Insert = 1,
        MA_Grab = 2,
        MA_MoveStartPoint,
        MA_MoveEndPoint,
        MA_ResizeNW = 8,
        MA_ResizeN,
        MA_ResizeNE,
        MA_ResizeE,
        MA_ResizeSE,
        MA_ResizeS,
        MA_ResizeSW,
        MA_ResizeW
    };

    int selected_x_offset;
    int selected_y_offset;

    ReportWindow * selected_items_rw;

    MouseAction mouseAction;
    KRObjectData::EntityTypes insertItem;

    QList<ReportEntity*> copy_list;
    QList<ReportEntity*> cut_list;
};

//! @internal
class KoReportDesigner::Private
{
public:
    QGridLayout *grid;
    QGraphicsScene *activeScene;
    KoRuler *hruler;
    KoZoomHandler *zoom;
    QVBoxLayout *vboxlayout;
    ReportPropertiesButton *pageButton;
};

KoReportDesigner::KoReportDesigner(QWidget * parent)
        : QWidget(parent), d(new Private())
{
    m_kordata = 0;
    init();
}

void KoReportDesigner::init()
{
    m_modified = false;
    m_detail = 0;
    d->hruler = 0;

    m_sectionData = new ReportWriterSectionData();
    createProperties();

    m_reportHeader = m_reportFooter = 0;
    m_pageHeaderFirst = m_pageHeaderOdd = m_pageHeaderEven = m_pageHeaderLast = m_pageHeaderAny = 0;
    m_pageFooterFirst = m_pageFooterOdd = m_pageFooterEven = m_pageFooterLast = m_pageFooterAny = 0;

    d->grid = new QGridLayout(this);
    d->grid->setSpacing(0);
    d->grid->setMargin(0);
    d->grid->setColumnStretch(1, 1);
    d->grid->setRowStretch(1, 1);
    d->grid->setSizeConstraint(QLayout::SetFixedSize);

    d->vboxlayout = new QVBoxLayout();
    d->vboxlayout->setSpacing(0);
    d->vboxlayout->setMargin(0);
    d->vboxlayout->setSizeConstraint(QLayout::SetFixedSize);

    //Create nice rulers
    d->zoom = new KoZoomHandler();
    d->hruler = new KoRuler(this, Qt::Horizontal, d->zoom);

    d->pageButton = new ReportPropertiesButton(this);

    //Messy, but i cant find another way
    delete d->hruler->tabChooser();
    d->hruler->setUnit(KoUnit(KoUnit::Centimeter));

    d->grid->addWidget(d->pageButton, 0, 0);
    d->grid->addWidget(d->hruler, 0, 1);
    d->grid->addLayout(d->vboxlayout, 1, 0, 1, 2);

    d->pageButton->setMaximumSize(QSize(19, 22));
    d->pageButton->setMinimumSize(QSize(19, 22));

    m_detail = new ReportSectionDetail(this);
    d->vboxlayout->insertWidget(0, m_detail);

    setLayout(d->grid);

    connect(d->pageButton, SIGNAL(released()), this, SLOT(slotPageButton_Pressed()));
    emit pagePropertyChanged(*m_set);

    connect(m_set, SIGNAL(propertyChanged(KoProperty::Set&, KoProperty::Property&)),
            this, SLOT(slotPropertyChanged(KoProperty::Set&, KoProperty::Property&)));

    changeSet(m_set);
}

KoReportDesigner::~KoReportDesigner()
{
}

///The loading Code
KoReportDesigner::KoReportDesigner(QWidget *parent, QDomElement data) : QWidget(parent), d(new Private())
{
    m_kordata = 0;

    init();

    if (data.tagName() != "report:content") {
        // arg we got an xml file but not one i know of
        kDebug() << "root element was not <report:content>";;
    }

    kDebug() << data.text();
    
    deleteDetail();

    QDomNodeList nlist = data.childNodes();
    QDomNode it;

    for (int i = 0; i < nlist.count(); ++i) {
        it = nlist.item(i);
        // at this level all the children we get should be Elements
        if (it.isElement()) {
            QString n = it.nodeName().toLower();
            kDebug() << n;
            if (n == "report:title") {
                setReportTitle(it.firstChild().nodeValue());
            } else if (n == "report:script") {
                m_interpreter->setValue(it.toElement().attribute("report:script-interpreter"));
                m_script->setValue(it.firstChild().nodeValue());
            } else if (n == "report:grid") {
                m_showGrid->setValue(it.toElement().attribute("report:grid-visible", QString::number(1)).toInt() != 0);
                m_gridSnap->setValue(it.toElement().attribute("report:grid-snap", QString::number(1)).toInt() != 0);
                m_gridDivisions->setValue(it.toElement().attribute("report:grid-divisions", QString::number(4)).toInt());
                m_unit->setValue(it.toElement().attribute("report:page-unit", "cm"));
            }

            //TODO Load page options
            else if (n == "report:page-style") {
                QString pagetype = it.firstChild().nodeValue();

                if (pagetype == "predefined") {
                    m_pageSize->setValue(it.toElement().attribute("report:page-size", "A4"));
                } else if (pagetype == "custom") {
                    m_pageSize->setValue("custom");
                    m_customHeight->setValue(it.toElement().attribute("report:custom-page-height", "").toDouble());
                    m_leftMargin->setValue(it.toElement().attribute("report:custom-page-widtht", "").toDouble());
                } else if (pagetype == "label") {
                    //TODO
                }

                m_rightMargin->setValue(KoUnit::parseValue(it.toElement().attribute("fo:margin-right", "1.0cm")));
                m_leftMargin->setValue(KoUnit::parseValue(it.toElement().attribute("fo:margin-left", "1.0cm")));
                m_topMargin->setValue(KoUnit::parseValue(it.toElement().attribute("fo:margin-top", "1.0cm")));
                m_bottomMargin->setValue(KoUnit::parseValue(it.toElement().attribute("fo:margin-bottom", "1.0cm")));

                m_orientation->setValue(it.toElement().attribute("report:print-orientation", "portrait"));

            } else if (n == "report:body") {
                QDomNodeList sectionlist = it.childNodes();
                QDomNode sec;

                for (int s = 0; s < sectionlist.count(); ++s) {
                    sec = sectionlist.item(s);
                    if (sec.isElement()) {
                        QString sn = sec.nodeName().toLower();
                        kDebug() << sn;
                        if (sn == "report:section") {
                            QString sectiontype = sec.toElement().attribute("report:section-type");
                            if (section(KRSectionData::sectionTypeFromString(sectiontype)) == 0) {
                                insertSection(KRSectionData::sectionTypeFromString(sectiontype));
                                section(KRSectionData::sectionTypeFromString(sectiontype))->initFromXML(sec);
                            }
                        } else if (sn == "report:detail") {
                            ReportSectionDetail * rsd = new ReportSectionDetail(this);
                            rsd->initFromXML(sec);
                            setDetail(rsd);
                        }
                    } else {
                        kDebug() << "Encountered an unknown Element: "  << n;
                    }
                }
            }
        } else {
            kDebug() << "Encountered a child node of root that is not an Element";
        }
    }
    this->slotPageButton_Pressed();
    emit(reportDataChanged());
    setModified(false);
}

///The saving code
QDomElement KoReportDesigner::document() const
{
    QDomDocument doc;

    QDomElement content = doc.createElement("report:content");
    content.setAttribute("xmlns:report", ns);
    content.setAttribute("xmlns:fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");
    content.setAttribute("xmlns:svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0");

    doc.appendChild(content);

    //title
    content.appendChild(propertyToElement(&doc, m_title));

    QDomElement scr = propertyToElement(&doc, m_script);
    ReportEntity::addPropertyAsAttribute(&scr, m_interpreter);
    content.appendChild(scr);

    QDomElement grd = doc.createElement("report:grid");
    ReportEntity::addPropertyAsAttribute(&grd, m_showGrid);
    ReportEntity::addPropertyAsAttribute(&grd, m_gridDivisions);
    ReportEntity::addPropertyAsAttribute(&grd, m_gridSnap);
    ReportEntity::addPropertyAsAttribute(&grd, m_unit);
    content.appendChild(grd);

    // pageOptions
    // -- size
    QDomElement pagestyle = doc.createElement("report:page-style");

    if (m_pageSize->value().toString() == "Custom") {
        pagestyle.appendChild(doc.createTextNode("custom"));
        pagestyle.setAttribute("report:page-width", QString::number(pageUnit().fromUserValue(m_customWidth->value().toDouble())));
        pagestyle.setAttribute("report:page-height", QString::number(pageUnit().fromUserValue(m_customWidth->value().toDouble())));

    } else if (m_pageSize->value().toString() == "Label") {
        pagestyle.appendChild(doc.createTextNode("label"));
        pagestyle.setAttribute("report:page-label-type", m_labelType->value().toString());
    } else {
        pagestyle.appendChild(doc.createTextNode("predefined"));
	ReportEntity::addPropertyAsAttribute(&pagestyle, m_pageSize);
        //pagestyle.setAttribute("report:page-size", m_pageSize->value().toString());
    }

    // -- orientation
    ReportEntity::addPropertyAsAttribute(&pagestyle, m_orientation);

    // -- margins
    pagestyle.setAttribute("fo:margin-top", KoUnit::unit(m_topMargin->option("unit").toString()).toUserStringValue(m_topMargin->value().toDouble()) + m_topMargin->option("unit").toString());
    pagestyle.setAttribute("fo:margin-bottom", KoUnit::unit(m_bottomMargin->option("unit").toString()).toUserStringValue(m_bottomMargin->value().toDouble()) + m_topMargin->option("unit").toString());
    pagestyle.setAttribute("fo:margin-right", KoUnit::unit(m_rightMargin->option("unit").toString()).toUserStringValue(m_rightMargin->value().toDouble()) + m_topMargin->option("unit").toString());
    pagestyle.setAttribute("fo:margin-left", KoUnit::unit(m_leftMargin->option("unit").toString()).toUserStringValue(m_leftMargin->value().toDouble()) + m_topMargin->option("unit").toString());

    content.appendChild(pagestyle);

    QDomElement body = doc.createElement("report:body");
    QDomElement domsection;

    for (int i = KRSectionData::PageHeaderFirst; i <= KRSectionData::PageFooterAny; ++i) {
        ReportSection *sec = section((KRSectionData::Section)i);
        if (sec) {
            domsection = doc.createElement("report:section");
            domsection.setAttribute("report:section-type", KRSectionData::sectionTypeString(KRSectionData::Section(i)));
            sec->buildXML(doc, domsection);
            body.appendChild(domsection);
        }
    }

    QDomElement detail = doc.createElement("report:detail");
    m_detail->buildXML(doc, detail);
    body.appendChild(detail);

    content.appendChild(body);
    return content;
}

void KoReportDesigner::slotSectionEditor()
{
    QPointer<SectionEditor> se = new SectionEditor(this);
    se->init(this);
    se->exec();
    delete se;
}

void KoReportDesigner::setReportData(KoReportData* kodata)
{
    kDebug();
    if (kodata) {
        m_kordata = kodata;
        slotPageButton_Pressed();
        setModified(true);
        emit(reportDataChanged());
    }
}

ReportSection * KoReportDesigner::section(KRSectionData::Section s) const
{
    ReportSection *sec;
    switch (s) {
    case KRSectionData::PageHeaderAny:
        sec = m_pageHeaderAny;
        break;
    case KRSectionData::PageHeaderEven:
        sec = m_pageHeaderEven;
        break;
    case KRSectionData::PageHeaderOdd:
        sec = m_pageHeaderOdd;
        break;
    case KRSectionData::PageHeaderFirst:
        sec = m_pageHeaderFirst;
        break;
    case KRSectionData::PageHeaderLast:
        sec = m_pageHeaderLast;
        break;
    case KRSectionData::PageFooterAny:
        sec = m_pageFooterAny;
        break;
    case KRSectionData::PageFooterEven:
        sec = m_pageFooterEven;
        break;
    case KRSectionData::PageFooterOdd:
        sec = m_pageFooterOdd;
        break;
    case KRSectionData::PageFooterFirst:
        sec = m_pageFooterFirst;
        break;
    case KRSectionData::PageFooterLast:
        sec = m_pageFooterLast;
        break;
    case KRSectionData::ReportHeader:
        sec = m_reportHeader;
        break;
    case KRSectionData::ReportFooter:
        sec = m_reportFooter;
        break;
    default:
        sec = 0;
    }
    return sec;
}
void KoReportDesigner::removeSection(KRSectionData::Section s)
{
    ReportSection* sec = section(s);
    if (sec) {
        delete sec;

        switch (s) {
        case KRSectionData::PageHeaderAny:
            m_pageHeaderAny = 0;
            break;
        case KRSectionData::PageHeaderEven:
            sec = m_pageHeaderEven = 0;
            break;
        case KRSectionData::PageHeaderOdd:
            m_pageHeaderOdd = 0;
            break;
        case KRSectionData::PageHeaderFirst:
            m_pageHeaderFirst = 0;
            break;
        case KRSectionData::PageHeaderLast:
            m_pageHeaderLast = 0;
            break;
        case KRSectionData::PageFooterAny:
            m_pageFooterAny = 0;
            break;
        case KRSectionData::PageFooterEven:
            m_pageFooterEven = 0;
            break;
        case KRSectionData::PageFooterOdd:
            m_pageFooterOdd = 0;
            break;
        case KRSectionData::PageFooterFirst:
            m_pageFooterFirst = 0;
            break;
        case KRSectionData::PageFooterLast:
            m_pageFooterLast = 0;
            break;
        case KRSectionData::ReportHeader:
            m_reportHeader = 0;
            break;
        case KRSectionData::ReportFooter:
            m_reportFooter = 0;
            break;
        default:
            sec = 0;
        }

        setModified(true);
        adjustSize();
    }
}

void KoReportDesigner::insertSection(KRSectionData::Section s)
{
    ReportSection* sec = section(s);
    if (!sec) {
        int idx = 0;
        for (int i = 1; i <= s; ++i) {
            if (section((KRSectionData::Section)i))
                idx++;
        }
        if (s > KRSectionData::ReportHeader)
            idx++;
        kDebug() << idx;
        ReportSection *rs = new ReportSection(this);
        d->vboxlayout->insertWidget(idx, rs);

        switch (s) {
        case KRSectionData::PageHeaderAny:
            rs->setTitle(i18n("Page Header (Any)"));
            m_pageHeaderAny = rs;
            break;
        case KRSectionData::PageHeaderEven:
            rs->setTitle(i18n("Page Header (Even)"));
            m_pageHeaderEven = rs;
            break;
        case KRSectionData::PageHeaderOdd:
            rs->setTitle(i18n("Page Header (Odd)"));
            m_pageHeaderOdd = rs;
            break;
        case KRSectionData::PageHeaderFirst:
            rs->setTitle(i18n("Page Header (First)"));
            m_pageHeaderFirst = rs;
            break;
        case KRSectionData::PageHeaderLast:
            rs->setTitle(i18n("Page Header (Last)"));
            m_pageHeaderLast = rs;
            break;
        case KRSectionData::PageFooterAny:
            rs->setTitle(i18n("Page Footer (Any)"));
            m_pageFooterAny = rs;
            break;
        case KRSectionData::PageFooterEven:
            rs->setTitle(i18n("Page Footer (Even)"));
            m_pageFooterEven = rs;
            break;
        case KRSectionData::PageFooterOdd:
            rs->setTitle(i18n("Page Footer (Odd)"));
            m_pageFooterOdd = rs;
            break;
        case KRSectionData::PageFooterFirst:
            rs->setTitle(i18n("Page Footer (First)"));
            m_pageFooterFirst = rs;
            break;
        case KRSectionData::PageFooterLast:
            rs->setTitle(i18n("Page Footer (Last)"));
            m_pageFooterLast = rs;
            break;
        case KRSectionData::ReportHeader:
            rs->setTitle(i18n("Report Header"));
            m_reportHeader = rs;
            break;
        case KRSectionData::ReportFooter:
            rs->setTitle(i18n("Report Footer"));
            m_reportFooter = rs;
            break;
            //These sections cannot be inserted this way
        case KRSectionData::None:
        case KRSectionData::GroupHeader:
        case KRSectionData::GroupFooter:
        case KRSectionData::Detail:
            break;
        }

        rs->show();
        setModified(true);
        adjustSize();
        emit pagePropertyChanged(*m_set);
    }
}

void KoReportDesigner::setReportTitle(const QString & str)
{
    if (reportTitle() != str) {
        m_title->setValue(str);
        setModified(true);
    }
}
QString KoReportDesigner::reportTitle() const
{
    return m_title->value().toString();
}

bool KoReportDesigner::isModified() const
{
    return m_modified;
}

void KoReportDesigner::setModified(bool mod)
{
    m_modified = mod;

    if (m_modified) {
        emit(dirty());
    }
}

QStringList KoReportDesigner::fieldNames() const
{
    QStringList qs;
    qs << QString();
    if (m_kordata)
        qs << m_kordata->fieldNames();

    return qs;
}

QStringList KoReportDesigner::fieldKeys() const
{
    QStringList qs;
    qs << QString();
    if (m_kordata)
        qs << m_kordata->fieldKeys();

    return qs;
}

void KoReportDesigner::createProperties()
{
    QStringList keys, strings;
    m_set = new KoProperty::Set(0, "Report");

    connect(m_set, SIGNAL(propertyChanged(KoProperty::Set&, KoProperty::Property&)),
            this, SLOT(slotPropertyChanged(KoProperty::Set&, KoProperty::Property&)));

    m_title = new KoProperty::Property("Title", "Report", i18n("Title"), i18n("Report Title"));

    keys.clear();
    keys = pageFormats();
    m_pageSize = new KoProperty::Property("page-size", keys, keys, "A4", i18n("Page Size"));

    keys.clear(); strings.clear();
    keys << "portrait" << "landscape";
    strings << i18n("Portrait") << i18n("Landscape");
    m_orientation = new KoProperty::Property("print-orientation", keys, strings, "portrait", i18n("Page Orientation"));

    keys.clear(); strings.clear();

    strings = KoUnit::listOfUnitName();
    QString unit;
    foreach(const QString &un, strings) {
        unit = un.mid(un.indexOf('(') + 1, 2);
        keys << unit;
    }

    m_unit = new KoProperty::Property("page-unit", keys, strings, "cm", i18n("Page Unit"));

    m_showGrid = new KoProperty::Property("grid-visible", true, i18n("Show Grid"), i18n("Show Grid"));
    m_gridSnap = new KoProperty::Property("grid-snap", true, i18n("Grid Snap"), i18n("Grid Snap"));
    m_gridDivisions = new KoProperty::Property("grid-divisions", 4, i18n("Grid Divisions"), i18n("Grid Divisions"));

    m_leftMargin = new KoProperty::Property("margin-left", KoUnit::unit("cm").fromUserValue(1.0),
        i18n("Left Margin"), i18n("Left Margin"), KoProperty::Double);
    m_rightMargin = new KoProperty::Property("margin-right", KoUnit::unit("cm").fromUserValue(1.0),
        i18n("Right Margin"), i18n("Right Margin"), KoProperty::Double);
    m_topMargin = new KoProperty::Property("margin-top", KoUnit::unit("cm").fromUserValue(1.0),
        i18n("Top Margin"), i18n("Top Margin"), KoProperty::Double);
    m_bottomMargin = new KoProperty::Property("margin-bottom", KoUnit::unit("cm").fromUserValue(1.0),
        i18n("Bottom Margin"), i18n("Bottom Margin"), KoProperty::Double);
    m_leftMargin->setOption("unit", "cm");
    m_rightMargin->setOption("unit", "cm");
    m_topMargin->setOption("unit", "cm");
    m_bottomMargin->setOption("unit", "cm");

    keys = Kross::Manager::self().interpreters();
    m_interpreter = new KoProperty::Property("script-interpreter", keys, keys, keys[0], i18n("Script Interpreter"));

    m_script = new KoProperty::Property("script", keys, keys, QString(), i18n("Object Script"));

    m_set->addProperty(m_title);
    m_set->addProperty(m_pageSize);
    m_set->addProperty(m_orientation);
    m_set->addProperty(m_unit);
    m_set->addProperty(m_gridSnap);
    m_set->addProperty(m_showGrid);
    m_set->addProperty(m_gridDivisions);
    m_set->addProperty(m_leftMargin);
    m_set->addProperty(m_rightMargin);
    m_set->addProperty(m_topMargin);
    m_set->addProperty(m_bottomMargin);
    m_set->addProperty(m_interpreter);
    m_set->addProperty(m_script);

//    KoProperty::Property* _customHeight;
//    KoProperty::Property* _customWidth;

}

/**
@brief Handle property changes
*/
void KoReportDesigner::slotPropertyChanged(KoProperty::Set &s, KoProperty::Property &p)
{
    setModified(true);
    emit pagePropertyChanged(s);

    if (p.name() == "page-unit") {
        d->hruler->setUnit(pageUnit());
        QString newstr = m_set->property("page-unit").value().toString();

        m_set->property("margin-left").setOption("unit", newstr);
        m_set->property("margin-right").setOption("unit", newstr);
        m_set->property("margin-top").setOption("unit", newstr);
        m_set->property("margin-bottom").setOption("unit", newstr);
    }
}

void KoReportDesigner::slotPageButton_Pressed()
{
    if (m_kordata) {
        QStringList sl = m_kordata->scriptList(m_interpreter->value().toString());

        m_script->setListData(sl, sl);
        changeSet(m_set);
    }
}

QStringList KoReportDesigner::pageFormats() const
{
    QStringList lst;
    lst << "A4" << "Letter" << "Legal" << "A3" << "A5";
    return lst;
}

QSize KoReportDesigner::sizeHint() const
{
    int w = 0;
    int h = 0;

    if (m_pageFooterAny)
        h += m_pageFooterAny->sizeHint().height();
    if (m_pageFooterEven)
        h += m_pageFooterEven->sizeHint().height();
    if (m_pageFooterFirst)
        h += m_pageFooterFirst->sizeHint().height();
    if (m_pageFooterLast)
        h += m_pageFooterLast->sizeHint().height();
    if (m_pageFooterOdd)
        h += m_pageFooterOdd->sizeHint().height();
    if (m_pageHeaderAny)
        h += m_pageHeaderAny->sizeHint().height();
    if (m_pageHeaderEven)
        h += m_pageHeaderEven->sizeHint().height();
    if (m_pageHeaderFirst)
        h += m_pageHeaderFirst->sizeHint().height();
    if (m_pageHeaderLast)
        h += m_pageHeaderLast->sizeHint().height();
    if (m_pageHeaderOdd)
        h += m_pageHeaderOdd->sizeHint().height();
    if (m_reportHeader)
        h += m_reportHeader->sizeHint().height();
    if (m_reportFooter) {
        h += m_reportFooter->sizeHint().height();

    }
    if (m_detail) {
        h += m_detail->sizeHint().height();
        w += m_detail->sizeHint().width();
    }

    h += d->hruler->height();

    return QSize(w, h);
}

int KoReportDesigner::pageWidthPx() const
{
    int cw = 0;
    int ch = 0;
    int width = 0;

    KoPageFormat::Format pf = KoPageFormat::formatFromString(m_set->property("page-size").value().toString());

    cw = POINT_TO_INCH(MM_TO_POINT(KoPageFormat::width(pf, KoPageFormat::Portrait))) * KoDpi::dpiX();

    ch = POINT_TO_INCH(MM_TO_POINT(KoPageFormat::height(pf, KoPageFormat::Portrait))) * KoDpi::dpiY();

    width = (m_set->property("print-orientation").value().toString() == "portrait" ? cw : ch);

    width = width - POINT_TO_INCH(m_set->property("margin-left").value().toDouble()) * KoDpi::dpiX();
    width = width - POINT_TO_INCH(m_set->property("margin-right").value().toDouble()) * KoDpi::dpiX();

    return width;
}

void KoReportDesigner::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event);
    //hruler->setRulerLength ( vboxlayout->geometry().width() );
    d->hruler->setRulerLength(pageWidthPx());
}

void KoReportDesigner::setDetail(ReportSectionDetail *rsd)
{
    if (!m_detail) {
        int idx = 0;
        if (m_pageHeaderFirst) idx++;
        if (m_pageHeaderOdd) idx++;
        if (m_pageHeaderEven) idx++;
        if (m_pageHeaderLast) idx++;
        if (m_pageHeaderAny) idx++;
        if (m_reportHeader) idx++;
        m_detail = rsd;
        d->vboxlayout->insertWidget(idx, m_detail);
    }
}
void KoReportDesigner::deleteDetail()
{
    if (m_detail) {
        delete m_detail;
        m_detail = 0;
    }
}

KoUnit KoReportDesigner::pageUnit() const
{
    QString u;
    bool found;

    u = m_unit->value().toString();

    KoUnit unit = KoUnit::unit(u, &found);
    if (!found) {
        unit = KoUnit::unit("cm");
    }

    return unit;
}

void KoReportDesigner::setGridOptions(bool vis, int div)
{
    m_showGrid->setValue(QVariant(vis));
    m_gridDivisions->setValue(div);
}

//
// methods for the sectionMouse*Event()
//
void KoReportDesigner::sectionContextMenuEvent(ReportScene * s, QGraphicsSceneContextMenuEvent * e)
{
    QMenu pop;
    QAction *popCut = 0;
    QAction *popCopy = 0;
    QAction *popPaste = 0;
    QAction* popDelete = 0;

    bool itemsSelected = selectionCount() > 0;
    if (itemsSelected) {
        popCut = pop.addAction(i18n("Cut"));
        popCopy = pop.addAction(i18n("Copy"));
    }
    if (!m_sectionData->copy_list.isEmpty())
        popPaste = pop.addAction(i18n("Paste"));

    if (itemsSelected) {
        pop.addSeparator();
        popDelete = pop.addAction(i18n("Delete"));
    }

    if (pop.actions().count() > 0) {
        QAction * ret = pop.exec(e->screenPos());
        if (ret == popCut)
            slotEditCut();
        else if (ret == popCopy)
            slotEditCopy();
        else if (ret == popPaste)
            slotEditPaste(s);
        else if (ret == popDelete)
            slotEditDelete();
    }
}

void KoReportDesigner::sectionMouseReleaseEvent(ReportSceneView * v, QMouseEvent * e)
{
    e->accept();
    QGraphicsItem * item = 0;
    if (e->button() == Qt::LeftButton) {
        QPointF pos(e->x(), e->y());
        switch (m_sectionData->mouseAction) {
        case ReportWriterSectionData::MA_Insert:
            switch (m_sectionData->insertItem) {
            case KRObjectData::EntityLabel :
                item = new ReportEntityLabel(v->designer(), v->scene(), pos);
                break;
            case KRObjectData::EntityField :
                item = new ReportEntityField(v->designer(), v->scene(), pos);
                break;
            case KRObjectData::EntityText :
                item = new ReportEntityText(v->designer(), v->scene(), pos);
                break;
            case KRObjectData::EntityBarcode :
                item = new ReportEntityBarcode(v->designer(), v->scene(), pos);
                break;
            case KRObjectData::EntityImage :
                item = new ReportEntityImage(v->designer(), v->scene(), pos);
                break;
            case KRObjectData::EntityLine :
                item = new ReportEntityLine(v->designer(), v->scene(), pos);
                break;
            case KRObjectData::EntityChart :
                item = new ReportEntityChart(v->designer(), v->scene(), pos);
                break;
            case KRObjectData::EntityShape :
                item = new ReportEntityShape(v->designer(), v->scene(), pos);
                break;
            case KRObjectData::EntityCheck :
                item = new ReportEntityCheck(v->designer(), v->scene(), pos);
                break;
            default:
                kDebug() << "attempted to insert an unknown item";;
            }
            if (item) {
                item->setVisible(true);
                if (v && v->designer())
                    v->designer()->setModified(true);
            }

            m_sectionData->mouseAction = ReportWriterSectionData::MA_None;
            m_sectionData->insertItem = KRObjectData::EntityNone;
            break;
        default:
            // what to do? Nothing
            // either we don't know what is going on
            // or everything has been done elsewhere
            break;
        }
    }
}

unsigned int KoReportDesigner::selectionCount() const
{
    if (activeScene())
        return activeScene()->selectedItems().count();
    else
        return 0;
}

void KoReportDesigner::changeSet(KoProperty::Set *s)
{
    //Set the checked state of the report proerties button
    if (s == m_set)
        d->pageButton->setCheckState(Qt::Checked);
    else
        d->pageButton->setCheckState(Qt::Unchecked);

    m_itmset = s;
    emit(propertySetChanged());
}

//
// Actions
//

void KoReportDesigner::slotItem(KRObjectData::EntityTypes typ)
{
    m_sectionData->mouseAction = ReportWriterSectionData::MA_Insert;
    m_sectionData->insertItem = typ;
}

void KoReportDesigner::slotItem(const QString &entity)
{
    if (entity == "action-insert-label") slotItem(KRObjectData::EntityLabel);
    if (entity == "action-insert-field") slotItem(KRObjectData::EntityField);
    if (entity == "action-insert-text") slotItem(KRObjectData::EntityText);
    if (entity == "action-insert-line") slotItem(KRObjectData::EntityLine);
    if (entity == "action-insert-barcode") slotItem(KRObjectData::EntityBarcode);
    if (entity == "action-insert-chart") slotItem(KRObjectData::EntityChart);
    if (entity == "action-insert-check") slotItem(KRObjectData::EntityCheck);
    if (entity == "action-insert-image") slotItem(KRObjectData::EntityImage);
    if (entity == "action-insert-shape") slotItem(KRObjectData::EntityShape);

}

void KoReportDesigner::slotEditDelete()
{
    QGraphicsItem * item = 0;
    while (selectionCount() > 0) {
        item = activeScene()->selectedItems()[0];
        if (item) {
            setModified(true);
            QGraphicsScene * scene = item->scene();
            delete item;
            scene->update();
            m_sectionData->mouseAction = ReportWriterSectionData::MA_None;
        }
    }
    activeScene()->selectedItems().clear();
    m_sectionData->selected_items_rw = 0;

    //TODO temporary
    //clears cut and copy lists to make sure we do not crash
    //if weve deleted something in the list
    //should really check if an item is in the list first
    //and remove it.
    m_sectionData->cut_list.clear();
    m_sectionData->copy_list.clear();
}

void KoReportDesigner::slotEditCut()
{
    if (selectionCount() > 0) {
        //First delete any items that are curerntly in the list
        //so as not to leak memory
        qDeleteAll(m_sectionData->cut_list);
        m_sectionData->cut_list.clear();

        QGraphicsItem * item = activeScene()->selectedItems().first();
        if (item) {
            m_sectionData->copy_list.clear();

            for (int i = 0; i < activeScene()->selectedItems().count(); i++) {
                QGraphicsItem *itm = activeScene()->selectedItems()[i];
                m_sectionData->cut_list.append(dynamic_cast<ReportEntity*>(itm));
                m_sectionData->copy_list.append(dynamic_cast<ReportEntity*>(itm));
            }
            int c = activeScene()->selectedItems().count();
            for (int i = 0; i < c; i++) {
                QGraphicsItem *itm = activeScene()->selectedItems()[0];
                activeScene()->removeItem(itm);
                activeScene()->update();
            }
            m_sectionData->selected_x_offset = 10;
            m_sectionData->selected_y_offset = 10;
        }
    }
}

void KoReportDesigner::slotEditCopy()
{
    if (selectionCount() < 1)
        return;

    QGraphicsItem * item = activeScene()->selectedItems().first();
    if (item) {
        m_sectionData->copy_list.clear();

        for (int i = 0; i < activeScene()->selectedItems().count(); i++) {
            m_sectionData->copy_list.append(dynamic_cast<ReportEntity*>(activeScene()->selectedItems()[i]));
        }
        m_sectionData->selected_x_offset = 10;
        m_sectionData->selected_y_offset = 10;
    }
}

void KoReportDesigner::slotEditPaste()
{
    // call the editPaste function passing it a reportsection
    slotEditPaste(activeScene());
}

void KoReportDesigner::slotEditPaste(QGraphicsScene * canvas)
{
    // paste a new item of the copy we have in the specified location
    if (!m_sectionData->copy_list.isEmpty()) {
        QGraphicsItem * pasted_ent = 0;
        canvas->clearSelection();
        m_sectionData->mouseAction = ReportWriterSectionData::MA_None;

        //!TODO this code sucks :)
        //!The setPos calls only work AFTER the name has been set ?!?!?
        
        for (int i = 0; i < m_sectionData->copy_list.count(); i++) {
            pasted_ent = 0;
            int type = dynamic_cast<KRObjectData*>(m_sectionData->copy_list[i])->type();
            kDebug() << type;
            QPointF o(m_sectionData->selected_x_offset, m_sectionData->selected_y_offset);
            if (type == KRObjectData::EntityLabel) {
                ReportEntityLabel * ent = dynamic_cast<ReportEntityLabel*>(m_sectionData->copy_list[i])->clone();
                ent->setEntityName(suggestEntityName("label"));
                ent->setPos(ent->pos() + o);
                pasted_ent = ent;
            } else if (type == KRObjectData::EntityField) {
                ReportEntityField * ent = dynamic_cast<ReportEntityField*>(m_sectionData->copy_list[i])->clone();
                ent->setEntityName(suggestEntityName("field"));
                ent->setPos(ent->pos() + o);
                pasted_ent = ent;
            } else if (type == KRObjectData::EntityText) {
                ReportEntityText * ent = dynamic_cast<ReportEntityText*>(m_sectionData->copy_list[i])->clone();
                ent->setEntityName(suggestEntityName("text"));
                ent->setPos(ent->pos() + o);
                pasted_ent = ent;
            } else if (type == KRObjectData::EntityLine) {
                ReportEntityLine * ent = dynamic_cast<ReportEntityLine*>(m_sectionData->copy_list[i])->clone();
                ent->setEntityName(suggestEntityName("line"));
                ent->setLineScene(QLineF(ent->line().p1() + o, ent->line().p2() + o));
                pasted_ent = ent;
            } else if (type == KRObjectData::EntityBarcode) {
                ReportEntityBarcode * ent = dynamic_cast<ReportEntityBarcode*>(m_sectionData->copy_list[i])->clone();
                ent->setEntityName(suggestEntityName("barcode"));
                ent->setPos(ent->pos() + o);
                pasted_ent = ent;
            } else if (type == KRObjectData::EntityImage) {
                ReportEntityImage * ent = dynamic_cast<ReportEntityImage*>(m_sectionData->copy_list[i])->clone();
                ent->setEntityName(suggestEntityName("image"));
                ent->setPos(ent->pos() + o);
                pasted_ent = ent;
            } else if (type == KRObjectData::EntityCheck) {
                ReportEntityCheck * ent = dynamic_cast<ReportEntityCheck*>(m_sectionData->copy_list[i])->clone();
                ent->setEntityName(suggestEntityName("check"));
                ent->setPos(ent->pos() + o);
                pasted_ent = ent;
            }
            //TODO add graph
            //else if(cp.copy_item == ReportWriterSectionData::GraphItem) {
            //    ReportEntityGraph * ent = new ReportEntityGraph(cp.copy_graph, rw, canvas);
            //    ent->setX(pos.x() + cp.copy_offset_x);
            //    ent->setY(pos.y() + cp.copy_offset_y);
            //    ent->setSize(cp.copy_rect.width(), cp.copy_rect.height());
            //    ent->show();
            //    pasted_ent = ent;
            //}
            else {
                kDebug() << "Tried to paste an item I don't understand.";
            }

            if (pasted_ent) {
                canvas->addItem(pasted_ent);
                pasted_ent->show();
                m_sectionData->mouseAction = ReportWriterSectionData::MA_Grab;
                setModified(true);
            }
        }
        m_sectionData->selected_x_offset += 10;
        m_sectionData->selected_y_offset += 10;
        
    }
}
void KoReportDesigner::slotRaiseSelected()
{
    dynamic_cast<ReportScene*>(activeScene())->raiseSelected();
}

void KoReportDesigner::slotLowerSelected()
{
    dynamic_cast<ReportScene*>(activeScene())->lowerSelected();
}

QGraphicsScene* KoReportDesigner::activeScene() const
{
    return d->activeScene;
}

void KoReportDesigner::setActiveScene(QGraphicsScene* a)
{
    if (d->activeScene && d->activeScene != a)
        d->activeScene->clearSelection();
    d->activeScene = a;

    //Trigger an update so that the last scene redraws its title;
    update();
}

KoZoomHandler* KoReportDesigner::zoomHandler() const
{
    return d->zoom;
}

QString KoReportDesigner::suggestEntityName(const QString &n) const
{
    ReportSection *sec;
    int itemCount = 0;
    //Count items in the main sections
    for (int i = 1; i <= KRSectionData::PageFooterAny; i++) {
        sec = section((KRSectionData::Section) i);
        if (sec) {
            const QGraphicsItemList l = sec->items();
            itemCount += l.count();
        }
    }

    //Count items in the group headers/footers
    for (int i = 0; i < m_detail->groupSectionCount(); i++) {
        sec = m_detail->groupSection(i)->groupHeader();
        if (sec) {
            const QGraphicsItemList l = sec->items();
            itemCount += l.count();
        }
        sec = m_detail->groupSection(i)->groupFooter();
        if (sec) {
            const QGraphicsItemList l = sec->items();
            itemCount += l.count();
        }
    }

    if (m_detail) {
        sec = m_detail->detailSection();
        if (sec) {
            const QGraphicsItemList l = sec->items();
            itemCount += l.count();
        }
    }

    while (!isEntityNameUnique(n + QString::number(itemCount))) {
        itemCount++;
    }
    return n + QString::number(itemCount);
}

bool KoReportDesigner::isEntityNameUnique(const QString &n, KRObjectData* ignore) const
{
    ReportSection *sec;
    bool unique = true;

    //Check items in the main sections
    for (int i = 1; i <= KRSectionData::PageFooterAny; i++) {
        sec = section((KRSectionData::Section)i);
        if (sec) {
            const QGraphicsItemList l = sec->items();
            for (QGraphicsItemList::const_iterator it = l.constBegin(); it != l.constEnd(); ++it) {
                KRObjectData* itm = dynamic_cast<KRObjectData*>(*it);
                if (itm->entityName() == n  && itm != ignore) {
                    unique = false;
                    break;
                }
            }
            if (!unique) break;
        }
    }

    //Count items in the group headers/footers
    if (unique) {
        for (int i = 0; i < m_detail->groupSectionCount(); ++i) {
            sec = m_detail->groupSection(i)->groupHeader();
            if (sec) {
                const QGraphicsItemList l = sec->items();
                for (QGraphicsItemList::const_iterator it = l.constBegin(); it != l.constEnd(); ++it) {
                    KRObjectData* itm = dynamic_cast<KRObjectData*>(*it);
                    if (itm->entityName() == n  && itm != ignore) {
                        unique = false;
                        break;
                    }
                }

            }
            sec = m_detail->groupSection(i)->groupFooter();
            if (unique && sec) {
                const QGraphicsItemList l = sec->items();
                for (QGraphicsItemList::const_iterator it = l.constBegin(); it != l.constEnd(); ++it) {
                    KRObjectData* itm = dynamic_cast<KRObjectData*>(*it);
                    if (itm->entityName() == n  && itm != ignore) {
                        unique = false;
                        break;
                    }
                }
            }
        }
    }
    if (unique && m_detail) {
        sec = m_detail->detailSection();
        if (sec) {
            const QGraphicsItemList l = sec->items();
            for (QGraphicsItemList::const_iterator it = l.constBegin(); it != l.constEnd(); ++it) {
                KRObjectData* itm = dynamic_cast<KRObjectData*>(*it);
                if (itm->entityName() == n  && itm != ignore) {
                    unique = false;
                    break;
                }
            }
        }
    }

    return unique;
}

//static
QList<QAction*> KoReportDesigner::actions()
{
    QList<QAction*> actList;
    QAction *act;

    act = new QAction(KIcon("feed-subscribe"), i18n("Label"), 0);
    act->setObjectName("action-insert-label");
    actList << act;

    act = new QAction(KIcon("edit-rename"), i18n("Field"), 0);
    act->setObjectName("action-insert-field");
    actList << act;

    act = new QAction(KIcon("insert-text"), i18n("Text"), 0);
    act->setObjectName("action-insert-text");
    actList << act;

    act = new QAction(KIcon("draw-freehand"), i18n("Line"), 0);
    act->setObjectName("action-insert-line");
    actList << act;

    act = new QAction(KIcon("insert-barcode"), i18n("Barcode"), 0);
    act->setObjectName("action-insert-barcode");
    actList << act;

    act = new QAction(KIcon("insert-image"), i18n("Image"), 0);
    act->setObjectName("action-insert-image");
    actList << act;

    act = new QAction(KIcon("office-chart-area"), i18n("Chart"), 0);
    act->setObjectName("action-insert-chart");
    actList << act;

    act = new QAction(KIcon("view-statistics"), i18n("Shape"), 0);
    act->setObjectName("action-insert-shape");
    actList << act;

    act = new QAction(KIcon("draw-cross"), i18n("Check"), 0);
    act->setObjectName("action-insert-check");
    actList << act;

    return actList;

}

