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

#include "KoReportDesignerItemBase.h"

// qt
#include <QPainter>
#include <QString>
#include <QLabel>
#include <QLineEdit>
#include <QDomDocument>
#include <QInputDialog>
#include <QSlider>
#include <QDataStream>
#include <QCheckBox>
#include <QRadioButton>
#include <QSettings>
#include <kdebug.h>

#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <koproperty/EditorView.h>
#include <KoGlobal.h>
#include "KoReportItemBase.h"
#include <krutils.h>
#include <klocalizedstring.h>

//
// ReportEntity
//
KoReportDesignerItemBase::KoReportDesignerItemBase(KoReportDesigner* r)
{
    m_reportDesigner = r;
}

void KoReportDesignerItemBase::buildXML(QGraphicsItem * item, QDomDocument & doc, QDomElement & parent)
{
    KoReportDesignerItemBase *re = 0;
    re = dynamic_cast<KoReportDesignerItemBase*>(item);

    if (re) {
        re->buildXML(doc, parent);
    }

}

void KoReportDesignerItemBase::buildXMLRect(QDomDocument & doc, QDomElement & entity, KRPos *pos, KRSize *siz)
{
    Q_UNUSED(doc)
    KoUnit unit = pos->unit();
    
    entity.setAttribute("svg:x", QString::number(pos->toUnit().x()) + KoUnit::unitName(unit));
    entity.setAttribute("svg:y", QString::number(pos->toUnit().y()) + KoUnit::unitName(unit));
    entity.setAttribute("svg:width", QString::number(siz->toUnit().width()) + KoUnit::unitName(unit));
    entity.setAttribute("svg:height", QString::number(siz->toUnit().height()) + KoUnit::unitName(unit));
}

void KoReportDesignerItemBase::buildXMLTextStyle(QDomDocument & doc, QDomElement & entity, KRTextStyleData ts)
{
    QDomElement element = doc.createElement("report:text-style");

    element.setAttribute("fo:background-color", ts.backgroundColor.name());
    element.setAttribute("fo:foreground-color", ts.foregroundColor.name());
    element.setAttribute("fo:background-opacity", QString::number(ts.backgroundOpacity) + '%');
    KRUtils::writeFontAttributes(element, ts.font);

    entity.appendChild(element);
}

void KoReportDesignerItemBase::buildXMLLineStyle(QDomDocument & doc, QDomElement & entity, KRLineStyleData ls)
{
    QDomElement element = doc.createElement("report:line-style");

    element.setAttribute("report:line-color", ls.lineColor.name());
    element.setAttribute("report:line-weight", QString::number(ls.weight));

    QString l;
    switch (ls.style) {
    case Qt::NoPen:
        l = "nopen";
        break;
    case Qt::SolidLine:
        l = "solid";
        break;
    case Qt::DashLine:
        l = "dash";
        break;
    case Qt::DotLine:
        l = "dot";
        break;
    case Qt::DashDotLine:
        l = "dashdot";
        break;
    case Qt::DashDotDotLine:
        l = "dashdotdot";
        break;
    default:
        l = "solid";

    }
    element.setAttribute("report:line-style", l);

    entity.appendChild(element);
}

QString KoReportDesignerItemBase::dataSourceAndObjectTypeName(const QString& dataSource, const QString& objectTypeName)
{
    return i18nc("<data-source>: <object>", "%1: %2", dataSource, objectTypeName);
}

// static
void KoReportDesignerItemBase::addPropertyAsAttribute(QDomElement* e, KoProperty::Property* p)
{
    switch (p->value().type()) {
    case QVariant::Int :
        e->setAttribute(QLatin1String("report:") + p->name().toLower(), p->value().toInt());
        break;
    case QVariant::Double:
        e->setAttribute(QLatin1String("report:") + p->name().toLower(), p->value().toDouble());
        break;
    case QVariant::Bool:
        e->setAttribute(QLatin1String("report:") + p->name().toLower(), p->value().toInt());
        break;
    default:
        e->setAttribute(QLatin1String("report:") + p->name().toLower(), p->value().toString());
        break;
    }
}
