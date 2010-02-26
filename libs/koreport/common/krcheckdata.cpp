/*
 * Kexi Report Plugin
 * Copyright (C) 2009-2010 by Adam Pigg (adam@piggz.co.uk)
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

#include "krcheckdata.h"
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <KoGlobal.h>
#include <kdebug.h>
#include <klocalizedstring.h>

KRCheckData::KRCheckData(QDomNode &element)
{
    createProperties();
    QDomNodeList nl = element.childNodes();
    QString n;
    QDomNode node;

    m_name->setValue(element.toElement().attribute("report:name"));
    m_controlSource->setValue(element.toElement().attribute("report:control-source"));
    Z = element.toElement().attribute("report:z-index").toDouble();
    m_foregroundColor->setValue(element.toElement().attribute("fo:foreground-color"));
    m_checkStyle->setValue(element.toElement().attribute("report:check-style"));

    parseReportRect(element.toElement(), &m_pos, &m_size);

    for (int i = 0; i < nl.count(); i++) {
        node = nl.item(i);
        n = node.nodeName();

        if (n == "report:line-style") {
            KRLineStyleData ls;
            if (parseReportLineStyleData(node.toElement(), ls)) {
                m_lineWeight->setValue(ls.weight);
                m_lineColor->setValue(ls.lineColor);
                m_lineStyle->setValue(ls.style);
            }
        } else {
            kDebug() << "while parsing check element encountered unknow element: " << n;
        }
    }

}

KRCheckData::~KRCheckData()
{
    //dtor
}

void KRCheckData::createProperties()
{
    m_set = new KoProperty::Set(0, "Check");

    QStringList keys, strings;

    keys << "Cross" << "Tick" << "Dot";
    strings << i18n("Cross") << i18n("Tick") << i18n("Dot");
    m_checkStyle = new KoProperty::Property("check-style", keys, strings, "Cross", i18n("Style"));

    m_controlSource = new KoProperty::Property("control-source", QStringList(), QStringList(), QString(), i18n("Control Source"));
    m_controlSource->setOption("extraValueAllowed", "true");

    m_foregroundColor = new KoProperty::Property("foreground-color", Qt::black, i18n("Foreground Color"));

    m_lineWeight = new KoProperty::Property("line-weight", 1, i18n("Line Weight"));
    m_lineColor = new KoProperty::Property("line-color", Qt::black, i18n("Line Color"));
    m_lineStyle = new KoProperty::Property("line-style", Qt::SolidLine, i18n("Line Style"), i18n("Line Style"), KoProperty::LineStyle);

    addDefaultProperties();
    m_set->addProperty(m_controlSource);
    m_set->addProperty(m_checkStyle);
    m_set->addProperty(m_foregroundColor);
    m_set->addProperty(m_lineWeight);
    m_set->addProperty(m_lineColor);
    m_set->addProperty(m_lineStyle);
}

KRLineStyleData KRCheckData::lineStyle()
{
    KRLineStyleData ls;
    ls.weight = m_lineWeight->value().toInt();
    ls.lineColor = m_lineColor->value().value<QColor>();
    ls.style = (Qt::PenStyle)m_lineStyle->value().toInt();
    return ls;
}

QString KRCheckData::controlSource() const
{
    return m_controlSource->value().toString();
}
// RTTI
int KRCheckData::type() const
{
    return RTTI;
}
int KRCheckData::RTTI = KRObjectData::EntityCheck;
KRCheckData * KRCheckData::toCheck()
{
    return this;
}
