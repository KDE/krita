/*
 * Kexi Report Plugin
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

#include "KoReportItemBarcode.h"
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <KoGlobal.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kglobalsettings.h>

#include "barcodes.h"

KoReportItemBarcode::KoReportItemBarcode(QDomNode & element)
{
    createProperties();
    QDomNodeList nl = element.childNodes();
    QString n;
    QDomNode node;

    m_name->setValue(element.toElement().attribute("report:name"));
    m_controlSource->setValue(element.toElement().attribute("report:item-data-source"));
    Z = element.toElement().attribute("report:z-index").toDouble();
    m_horizontalAlignment->setValue(element.toElement().attribute("report:horizontal-align"));
    m_maxLength->setValue(element.toElement().attribute("report:barcode-max-length"));
    m_format->setValue(element.toElement().attribute("report:barcode-format"));
    parseReportRect(element.toElement(), &m_pos, &m_size);

}

void KoReportItemBarcode::setMaxLength(int i)
{
    if (i > 0) {
        if (m_maxLength->value().toInt() != i) {
            m_maxLength->setValue(i);
        }
        if (m_format->value().toString() == "3of9") {
            int C = i; // number of characters
            int N = 2; // narrow mult for wide line
            int X = 1; // narrow line width
            int I = 1; // interchange line width
            m_minWidthData = (((C + 2) * ((3 * N) + 6) * X) + ((C + 1) * I)) / 100.0;
            m_minHeight = m_minWidthData * 0.15;
            /*if(min_height < 0.25)*/ m_minHeight = 0.25;
            m_minWidthTotal = m_minWidthData + 0.22; // added a little buffer to make sure we don't loose any
            // of our required quiet zone in conversions
        } else if (m_format->value().toString() == "3of9+") {
            int C = i * 2; // number of characters
            int N = 2; // narrow mult for wide line
            int X = 1; // 1px narrow line
            int I = 1; // 1px narrow line interchange
            m_minWidthData = (((C + 2) * ((3 * N) + 6) * X) + ((C + 1) * I)) / 100.0;
            m_minHeight = m_minWidthData * 0.15;
            /*if(min_height < 0.25)*/ m_minHeight = 0.25;
            m_minWidthTotal = m_minWidthData + 0.22; // added a little buffer to make sure we don't loose any
            // of our required quiet zone in conversions
        } else if (m_format->value().toString() == "128") {
            int C = i; // assuming 1:1 ratio of data passed in to data actually used in encoding
            int X = 1; // 1px wide
            m_minWidthData = (((11 * C) + 35) * X) / 100.0;       // assuming CODE A or CODE B
            m_minHeight = m_minWidthData * 0.15;
            /*if(min_height < 0.25)*/ m_minHeight = 0.25;
            m_minWidthTotal = m_minWidthData + 0.22; // added a little bugger to make sure we don't loose any
            // of our required quiet zone in conversions
        } else if (m_format->value().toString() == "upc-a") {
            m_minWidthData = 0.95;
            m_minWidthTotal = 1.15;
            m_minHeight = 0.25;
        } else if (m_format->value().toString() == "upc-e") {
            m_minWidthData = 0.52;
            m_minWidthTotal = 0.70;
            m_minHeight = 0.25;
        } else if (m_format->value().toString() == "ean13") {
            m_minWidthData = 0.95;
            m_minWidthTotal = 1.15;
            m_minHeight = 0.25;
        } else if (m_format->value().toString() == "ean8") {
            m_minWidthData = 0.67;
            m_minWidthTotal = 0.90;
            m_minHeight = 0.25;
        } else {
            kDebug() << "Unknown format encountered: " << m_format->value().toString();
        }
    }
}

void KoReportItemBarcode::createProperties()
{
    m_set = new KoProperty::Set(0, "Barcode");

    QStringList keys, strings;

    m_controlSource = new KoProperty::Property("item-data-source", QStringList(), QStringList(), QString(), i18n("Data Source"));

    keys << "left" << "center" << "right";
    strings << i18n("Left") << i18n("Center") << i18n("Right");
    m_horizontalAlignment = new KoProperty::Property("horizontal-align", keys, strings, "left", i18n("Horizontal Alignment"));

    keys.clear();
    strings.clear();
    keys << "3of9" << "3of9+" << "128" << "upc-a" << "upc-e" << "ean13" << "ean8";
    strings << "3of9" << "3of9+" << "128" << "upc-a" << "upc-e" << "ean13" << "ean8";
    m_format = new KoProperty::Property("barcode-format", keys, strings, "3of9", i18n("Barcode Format"));

    m_maxLength = new KoProperty::Property("barcode-max-length", 5, i18n("Max Length"), i18n("Maximum Barcode Length"));

    addDefaultProperties();
    m_set->addProperty(m_controlSource);
    m_set->addProperty(m_format);
    m_set->addProperty(m_horizontalAlignment);
    m_set->addProperty(m_maxLength);
}

KoReportItemBarcode::~KoReportItemBarcode()
{
}

int KoReportItemBarcode::alignment()
{
    QString a = m_horizontalAlignment->value().toString();

    if (a == "left")
        return 0;
    else if (a == "center")
        return 1;
    else if (a == "right")
        return 2;
    else
        return 0;
}

QString KoReportItemBarcode::itemDataSource() const
{
    return m_controlSource->value().toString();
}

QString KoReportItemBarcode::format()
{
    return m_format->value().toString();
}

int KoReportItemBarcode::maxLength()
{
    return m_maxLength->value().toInt();
}

void KoReportItemBarcode::setFormat(const QString& f)
{
    m_format->setValue(f);
}

void KoReportItemBarcode::setAlignment(int)
{
    //TODO Barcode alignment
}

//RTTI
QString KoReportItemBarcode::typeName() const
{
    return "barcode";
}

int KoReportItemBarcode::renderSimpleData(OROPage *page, OROSection *section, const QPointF &offset,
                                          const QVariant &data, KRScriptHandler *script)
{
    Q_UNUSED(section);
    Q_UNUSED(script);

    QPointF pos = m_pos.toScene();
    QSizeF size = m_size.toScene();
    pos += offset;

    QRectF rect = QRectF(pos, size);

    QString val = data.toString();

    if (page) {
      QString fmt = m_format->value().toString();
      int align = alignment();
      if (fmt == "3of9")
	  render3of9(page, rect, val, align);
      else if (fmt == "3of9+")
	  renderExtended3of9(page, rect, val, align);
      else if (fmt == "128")
	  renderCode128(page, rect, val, align);
      else if (fmt == "ean13")
	  renderCodeEAN13(page, rect, val, align);
      else if (fmt == "ean8")
	  renderCodeEAN8(page, rect, val, align);
      else if (fmt == "upc-a")
	  renderCodeUPCA(page, rect, val, align);
      else if (fmt == "upc-e")
	  renderCodeUPCE(page, rect, val, align);
      else {
	  kDebug() << "Unknown barcode format:" << fmt;
      }
    }
    return 0;
}
