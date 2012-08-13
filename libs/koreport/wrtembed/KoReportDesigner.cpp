/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC <info@openmfg.com>
 * Copyright (C) 2007-2010 by Adam Pigg <adam@piggz.co.uk>
 * Copyright (C) 2011 Jarosław Staniek <staniek@kde.org>
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

#include "reportsectiondetailgroup.h"
#include "reportpropertiesbutton.h"
#include "sectioneditor.h"
#include "reportsectiondetail.h"
#include "krutils.h"
#include "KoReportPluginInterface.h"
#include "KoReportDesignerItemLine.h"

#include "KoReportPluginManager.h"

#include <QLayout>
#include <QDomDocument>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QMessageBox>
#include <KStandardGuiItem>
#include <KGuiItem>
#include <KStandardAction>

#include <KoIcon.h>
#include <koproperty/EditorView.h>
#include <KoRuler.h>
#include <KoZoomHandler.h>
#include <KoDpi.h>
#include <KoPageFormat.h>
#include <kaction.h>
#include <KLocale>
#include <KDebug>
#include <KToggleAction>
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
        insertItem = QString();
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
    QString insertItem;

    QList<KoReportDesignerItemBase*> copy_list;
    QList<KoReportDesignerItemBase*> cut_list;
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
    delete d->zoom;
    delete d;
    delete m_sectionData;
    delete m_set;
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
                    m_customHeight->setValue(KoUnit::parseValue(it.toElement().attribute("report:custom-page-height", "")));
                    m_customWidth->setValue(KoUnit::parseValue(it.toElement().attribute("report:custom-page-widtht", "")));
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
    emit reportDataChanged();
    slotPropertyChanged(*m_set, *m_unit); // set unit for all items
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
    KRUtils::addPropertyAsAttribute(&scr, m_interpreter);
    content.appendChild(scr);

    QDomElement grd = doc.createElement("report:grid");
    KRUtils::addPropertyAsAttribute(&grd, m_showGrid);
    KRUtils::addPropertyAsAttribute(&grd, m_gridDivisions);
    KRUtils::addPropertyAsAttribute(&grd, m_gridSnap);
    KRUtils::addPropertyAsAttribute(&grd, m_unit);
    content.appendChild(grd);

    // pageOptions
    // -- size
    QDomElement pagestyle = doc.createElement("report:page-style");

    if (m_pageSize->value().toString() == "Custom") {
        pagestyle.appendChild(doc.createTextNode("custom"));
        KRUtils::setAttribute(pagestyle, "report:custom-page-width", m_customWidth->value().toDouble());
        KRUtils::setAttribute(pagestyle, "report:custom-page-height", m_customHeight->value().toDouble());

    } else if (m_pageSize->value().toString() == "Label") {
        pagestyle.appendChild(doc.createTextNode("label"));
        pagestyle.setAttribute("report:page-label-type", m_labelType->value().toString());
    } else {
        pagestyle.appendChild(doc.createTextNode("predefined"));
        KRUtils::addPropertyAsAttribute(&pagestyle, m_pageSize);
        //pagestyle.setAttribute("report:page-size", m_pageSize->value().toString());
    }

    // -- orientation
    KRUtils::addPropertyAsAttribute(&pagestyle, m_orientation);

    // -- margins: save as points, and not localized
    KRUtils::setAttribute(pagestyle, "fo:margin-top", m_topMargin->value().toDouble());
    KRUtils::setAttribute(pagestyle, "fo:margin-bottom", m_bottomMargin->value().toDouble());
    KRUtils::setAttribute(pagestyle, "fo:margin-right", m_rightMargin->value().toDouble());
    KRUtils::setAttribute(pagestyle, "fo:margin-left", m_leftMargin->value().toDouble());

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
        emit reportDataChanged();
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
        emit dirty();
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
    keys =  KoPageFormat::pageFormatNames();
    strings = KoPageFormat::localizedPageFormatNames();
    QString defaultKey = KoPageFormat::formatString(KoPageFormat::defaultFormat());
    m_pageSize = new KoProperty::Property("page-size", keys, strings, defaultKey, i18n("Page Size"));

    keys.clear(); strings.clear();
    keys << "portrait" << "landscape";
    strings << i18n("Portrait") << i18n("Landscape");
    m_orientation = new KoProperty::Property("print-orientation", keys, strings, "portrait", i18n("Page Orientation"));

    keys.clear(); strings.clear();

    strings = KoUnit::listOfUnitNameForUi(KoUnit::HidePixel);
    QString unit;
    foreach(const QString &un, strings) {
        unit = un.mid(un.indexOf('(') + 1, 2);
        keys << unit;
    }

    m_unit = new KoProperty::Property("page-unit", keys, strings, "cm", i18n("Page Unit"));

    m_showGrid = new KoProperty::Property("grid-visible", true, i18n("Show Grid"), i18n("Show Grid"));
    m_gridSnap = new KoProperty::Property("grid-snap", true, i18n("Grid Snap"), i18n("Grid Snap"));
    m_gridDivisions = new KoProperty::Property("grid-divisions", 4, i18n("Grid Divisions"), i18n("Grid Divisions"));

    m_leftMargin = new KoProperty::Property("margin-left", KoUnit(KoUnit::Centimeter).fromUserValue(1.0),
        i18n("Left Margin"), i18n("Left Margin"), KoProperty::Double);
    m_rightMargin = new KoProperty::Property("margin-right", KoUnit(KoUnit::Centimeter).fromUserValue(1.0),
        i18n("Right Margin"), i18n("Right Margin"), KoProperty::Double);
    m_topMargin = new KoProperty::Property("margin-top", KoUnit(KoUnit::Centimeter).fromUserValue(1.0),
        i18n("Top Margin"), i18n("Top Margin"), KoProperty::Double);
    m_bottomMargin = new KoProperty::Property("margin-bottom", KoUnit(KoUnit::Centimeter).fromUserValue(1.0),
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
    delete m_detail;
    m_detail = 0;
}

KoUnit KoReportDesigner::pageUnit() const
{
    QString u;
    bool found;

    u = m_unit->value().toString();

    KoUnit unit = KoUnit::fromSymbol(u, &found);
    if (!found) {
        unit = KoUnit(KoUnit::Centimeter);
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

    bool itemsSelected = selectionCount() > 0;
    if (itemsSelected) {
        QAction *a = KStandardAction::cut(this, SLOT(slotEditCut()), &pop);
        a->setShortcut(QKeySequence::UnknownKey); // shortcuts have no effect in the popup menu
        pop.addAction(a);
        a = KStandardAction::copy(this, SLOT(slotEditCopy()), &pop);
        a->setShortcut(QKeySequence::UnknownKey); // shortcuts have no effect in the popup menu
        pop.addAction(a);
    }
    if (!m_sectionData->copy_list.isEmpty()) {
        QAction *a = KStandardAction::paste(this, SLOT(slotEditPaste()), &pop);
        a->setShortcut(QKeySequence::UnknownKey); // shortcuts have no effect in the popup menu
        pop.addAction(a);
    }

    if (itemsSelected) {
        pop.addSeparator();
        const KGuiItem del = KStandardGuiItem::del();
        QAction *a = new KAction(del.icon(), del.text(), &pop);
        a->setToolTip(del.toolTip());
        connect(a, SIGNAL(activated()), SLOT(slotEditDelete()));
        pop.addAction(a);
    }
    if (!pop.actions().isEmpty()) {
        pop.exec(e->screenPos());
    }
}

void KoReportDesigner::sectionMouseReleaseEvent(ReportSceneView * v, QMouseEvent * e)
{
    e->accept();
    QGraphicsItem * item = 0;
    if (e->button() == Qt::LeftButton) {
        QPointF pos(e->x(), e->y());

        if (m_sectionData->mouseAction == ReportWriterSectionData::MA_Insert) {
            if (m_sectionData->insertItem == "report:line") {
                item = new KoReportDesignerItemLine(v->designer(), v->scene(), pos);
            }
            else {
                KoReportPluginManager* pluginManager = KoReportPluginManager::self();
                KoReportPluginInterface *plug = pluginManager->plugin(m_sectionData->insertItem);
                if (plug) {
                    QObject *obj = plug->createDesignerInstance(v->designer(), v->scene(), pos);
                    if (obj) {
                        item = dynamic_cast<QGraphicsItem*>(obj);
                    }
                }
                else {
                    kDebug() << "attempted to insert an unknown item";
                }
            }
            if (item) {
                item->setVisible(true);
                if (v && v->designer()) {
                    v->designer()->setModified(true);
                }
                emit itemInserted(m_sectionData->insertItem);
            }
                
            m_sectionData->mouseAction = ReportWriterSectionData::MA_None;
            m_sectionData->insertItem = QString();
            unsetSectionCursor();
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
    //Set the checked state of the report properties button
    if (s == m_set)
        d->pageButton->setCheckState(Qt::Checked);
    else
        d->pageButton->setCheckState(Qt::Unchecked);

    m_itmset = s;
    emit propertySetChanged();
}

//
// Actions
//

void KoReportDesigner::slotItem(const QString &entity)
{
    m_sectionData->mouseAction = ReportWriterSectionData::MA_Insert;
    m_sectionData->insertItem = entity;
    setSectionCursor(QCursor(Qt::CrossCursor));
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
                m_sectionData->cut_list.append(dynamic_cast<KoReportDesignerItemBase*>(itm));
                m_sectionData->copy_list.append(dynamic_cast<KoReportDesignerItemBase*>(itm));
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
            m_sectionData->copy_list.append(dynamic_cast<KoReportDesignerItemBase*>(activeScene()->selectedItems()[i]));
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
            QString type;

            KoReportItemBase *obj = dynamic_cast<KoReportItemBase*>(m_sectionData->copy_list[i]);
            if (obj) {
                type = obj->typeName();
            }
                
            kDebug() << type;

            KoReportDesignerItemBase *ent = (m_sectionData->copy_list[i])->clone();
            KoReportItemBase *new_obj = dynamic_cast<KoReportItemBase*>(ent);
            new_obj->setEntityName(suggestEntityName(type));    
            pasted_ent = dynamic_cast<QGraphicsItem*>(ent);

            if (pasted_ent) {
                canvas->addItem(pasted_ent);
                pasted_ent->show();
                m_sectionData->mouseAction = ReportWriterSectionData::MA_Grab;
                setModified(true);
            }
        }
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

bool KoReportDesigner::isEntityNameUnique(const QString &n, KoReportItemBase* ignore) const
{
    ReportSection *sec;
    bool unique = true;

    //Check items in the main sections
    for (int i = 1; i <= KRSectionData::PageFooterAny; i++) {
        sec = section((KRSectionData::Section)i);
        if (sec) {
            const QGraphicsItemList l = sec->items();
            for (QGraphicsItemList::const_iterator it = l.constBegin(); it != l.constEnd(); ++it) {
                KoReportItemBase* itm = dynamic_cast<KoReportItemBase*>(*it);
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
                    KoReportItemBase* itm = dynamic_cast<KoReportItemBase*>(*it);
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
                    KoReportItemBase* itm = dynamic_cast<KoReportItemBase*>(*it);
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
                KoReportItemBase* itm = dynamic_cast<KoReportItemBase*>(*it);
                if (itm->entityName() == n  && itm != ignore) {
                    unique = false;
                    break;
                }
            }
        }
    }

    return unique;
}

static bool actionPriortyLessThan(QAction* act1, QAction* act2)
{
    if (act1->data().toInt() > 0 && act1->data().toInt() > 0) {
        return act1->data().toInt() < act2->data().toInt();
    }
    return false;
}

QList<QAction*> KoReportDesigner::actions(QActionGroup* group)
{
    KoReportPluginManager* manager = KoReportPluginManager::self();
    QList<QAction*> actList = manager->actions();
    
    KToggleAction *act = new KToggleAction(koIcon("line"), i18n("Line"), group);
    act->setObjectName("report:line");
    act->setData(9);
    actList << act;

    qSort(actList.begin(), actList.end(), actionPriortyLessThan);
    int i = 0;

    //TODO maybe this is a bit hackish
    //It finds the first plugin based on the priority in userdata
    //The lowest oriority a plugin can have is 10
    //And inserts a separator before it.
    bool sepInserted = false;
    foreach(QAction *a, actList) {
        ++i;
        if (!sepInserted && a->data().toInt() >= 10) {
            QAction *sep = new QAction("separator", group);
            sep->setSeparator(true);
            actList.insert(i-1, sep);
            sepInserted = true;
        }
        else {
            group->addAction(a);
        }
    }
    
    return actList;
}

void KoReportDesigner::setSectionCursor(const QCursor& c)
{
    if (m_pageFooterAny)
        m_pageFooterAny->setSectionCursor(c);
    if (m_pageFooterEven)
        m_pageFooterEven->setSectionCursor(c);
    if (m_pageFooterFirst)
        m_pageFooterFirst->setSectionCursor(c);
    if (m_pageFooterLast)
        m_pageFooterLast->setSectionCursor(c);
    if (m_pageFooterOdd)
        m_pageFooterOdd->setSectionCursor(c);

    if (m_pageHeaderAny)
        m_pageHeaderAny->setSectionCursor(c);
    if (m_pageHeaderEven)
        m_pageHeaderEven->setSectionCursor(c);
    if (m_pageHeaderFirst)
        m_pageHeaderFirst->setSectionCursor(c);
    if (m_pageHeaderLast)
        m_pageHeaderLast->setSectionCursor(c);
    if (m_pageHeaderOdd)
        m_pageHeaderOdd->setSectionCursor(c);
    
    if (m_detail)
        m_detail->setSectionCursor(c);
}

void KoReportDesigner::unsetSectionCursor()
{
    if (m_pageFooterAny)
        m_pageFooterAny->unsetSectionCursor();
    if (m_pageFooterEven)
        m_pageFooterEven->unsetSectionCursor();
    if (m_pageFooterFirst)
        m_pageFooterFirst->unsetSectionCursor();
    if (m_pageFooterLast)
        m_pageFooterLast->unsetSectionCursor();
    if (m_pageFooterOdd)
        m_pageFooterOdd->unsetSectionCursor();
    
    if (m_pageHeaderAny)
        m_pageHeaderAny->unsetSectionCursor();
    if (m_pageHeaderEven)
        m_pageHeaderEven->unsetSectionCursor();
    if (m_pageHeaderFirst)
        m_pageHeaderFirst->unsetSectionCursor();
    if (m_pageHeaderLast)
        m_pageHeaderLast->unsetSectionCursor();
    if (m_pageHeaderOdd)
        m_pageHeaderOdd->unsetSectionCursor();
    
    if (m_detail)
        m_detail->unsetSectionCursor();
}
