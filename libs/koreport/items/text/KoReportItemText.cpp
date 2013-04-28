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
#include "KoReportItemText.h"
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <KoGlobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kglobalsettings.h>
#include <QPrinter>
#include <renderobjects.h>

KoReportItemText::KoReportItemText(QDomNode & element) : m_bottomPadding(0.0)
{
    QDomNodeList nl = element.childNodes();
    QString n;
    QDomNode node;

    createProperties();
    m_name->setValue(element.toElement().attribute("report:name"));
    m_controlSource->setValue(element.toElement().attribute("report:item-data-source"));
    Z = element.toElement().attribute("report:z-index").toDouble();
    m_horizontalAlignment->setValue(element.toElement().attribute("report:horizontal-align"));
    m_verticalAlignment->setValue(element.toElement().attribute("report:vertical-align"));
    m_bottomPadding = element.toElement().attribute("report:bottom-padding").toDouble();

    parseReportRect(element.toElement(), &m_pos, &m_size);

    for (int i = 0; i < nl.count(); i++) {
        node = nl.item(i);
        n = node.nodeName();

        if (n == "report:text-style") {
            KRTextStyleData ts;
            if (parseReportTextStyleData(node.toElement(), ts)) {
                m_backgroundColor->setValue(ts.backgroundColor);
                m_foregroundColor->setValue(ts.foregroundColor);
                m_backgroundOpacity->setValue(ts.backgroundOpacity);
                m_font->setValue(ts.font);

            }
        } else if (n == "report:line-style") {
            KRLineStyleData ls;
            if (parseReportLineStyleData(node.toElement(), ls)) {
                m_lineWeight->setValue(ls.weight);
                m_lineColor->setValue(ls.lineColor);
                m_lineStyle->setValue(ls.style);
            }
        } else {
            kDebug() << "while parsing field element encountered unknow element: " << n;
        }
    }

}

KoReportItemText::~KoReportItemText()
{
    delete m_set;
}

Qt::Alignment KoReportItemText::textFlags() const
{
    Qt::Alignment align;
    QString t;
    t = m_horizontalAlignment->value().toString();
    if (t == "center")
        align = Qt::AlignHCenter;
    else if (t == "right")
        align = Qt::AlignRight;
    else
        align = Qt::AlignLeft;

    t = m_verticalAlignment->value().toString();
    if (t == "center")
        align |= Qt::AlignVCenter;
    else if (t == "bottom")
        align |= Qt::AlignBottom;
    else
        align |= Qt::AlignTop;

    return align;
}

void KoReportItemText::createProperties()
{
    m_set = new KoProperty::Set(0, "Text");

    //connect ( set, SIGNAL ( propertyChanged ( KoProperty::Set &, KoProperty::Property & ) ), this, SLOT ( propertyChanged ( KoProperty::Set &, KoProperty::Property & ) ) );

    QStringList keys, strings;

    //_query = new KoProperty::Property ( "Query", QStringList(), QStringList(), "Data Source", "Query" );
    m_controlSource = new KoProperty::Property("item-data-source", QStringList(), QStringList(), QString(), i18n("Data Source"));

    keys << "left" << "center" << "right";
    strings << i18n("Left") << i18n("Center") << i18n("Right");
    m_horizontalAlignment = new KoProperty::Property("horizontal-align", keys, strings, "left", i18n("Horizontal Alignment"));

    keys.clear();
    strings.clear();
    keys << "top" << "center" << "bottom";
    strings << i18n("Top") << i18n("Center") << i18n("Bottom");
    m_verticalAlignment = new KoProperty::Property("vertical-align", keys, strings, "center", i18n("Vertical Alignment"));

    m_font = new KoProperty::Property("Font", KGlobalSettings::generalFont(), "Font", i18n("Font"));

    m_backgroundColor = new KoProperty::Property("background-color", Qt::white, i18n("Background Color"));
    m_foregroundColor = new KoProperty::Property("foreground-color", Qt::black, i18n("Foreground Color"));

    m_lineWeight = new KoProperty::Property("line-weight", 1, i18n("Line Weight"));
    m_lineColor = new KoProperty::Property("line-color", Qt::black, i18n("Line Color"));
    m_lineStyle = new KoProperty::Property("line-style", Qt::NoPen, i18n("Line Style"), i18n("Line Style"), KoProperty::LineStyle);
    m_backgroundOpacity = new KoProperty::Property("background-opacity", 100, i18n("Opacity"));
    m_backgroundOpacity->setOption("max", 100);
    m_backgroundOpacity->setOption("min", 0);
    m_backgroundOpacity->setOption("unit", "%");

    addDefaultProperties();
    m_set->addProperty(m_controlSource);
    m_set->addProperty(m_horizontalAlignment);
    m_set->addProperty(m_verticalAlignment);
    m_set->addProperty(m_font);
    m_set->addProperty(m_backgroundColor);
    m_set->addProperty(m_foregroundColor);
    m_set->addProperty(m_backgroundOpacity);
    m_set->addProperty(m_lineWeight);
    m_set->addProperty(m_lineColor);
    m_set->addProperty(m_lineStyle);

}

QString KoReportItemText::itemDataSource() const
{
    return m_controlSource->value().toString();
}

qreal KoReportItemText::bottomPadding() const
{
    return m_bottomPadding;
}

void KoReportItemText::setBottomPadding(qreal bp)
{
    if (m_bottomPadding != bp) {
        m_bottomPadding = bp;
    }
}

KRTextStyleData KoReportItemText::textStyle()
{
    KRTextStyleData d;
    d.backgroundColor = m_backgroundColor->value().value<QColor>();
    d.foregroundColor = m_foregroundColor->value().value<QColor>();
    d.font = m_font->value().value<QFont>();
    d.backgroundOpacity = m_backgroundOpacity->value().toInt();
    return d;
}

KRLineStyleData KoReportItemText::lineStyle()
{
    KRLineStyleData ls;
    ls.weight = m_lineWeight->value().toInt();
    ls.lineColor = m_lineColor->value().value<QColor>();
    ls.style = (Qt::PenStyle)m_lineStyle->value().toInt();
    return ls;
}

// RTTI
QString KoReportItemText::typeName() const
{
    return "report:text";
}

int KoReportItemText::renderSimpleData(OROPage *page, OROSection *section, const QPointF &offset,
                                       const QVariant &data, KRScriptHandler *script)

{
    Q_UNUSED(script);

    QString qstrValue;

    QString cs = itemDataSource();

    if (cs.left(1) == "$") { //Everything past $ is treated as a string
        qstrValue = cs.mid(1);
    } else {
        qstrValue = data.toString();
    }

    QPointF pos = m_pos.toScene();
    QSizeF size = m_size.toScene();
    pos += offset;

    QRectF trf(pos, size);

    int     intLineCounter  = 0;
    qreal   intStretch      = trf.top() - offset.y();
    qreal   intBaseTop      = trf.top();
    qreal   intRectHeight   = trf.height();

    kDebug() << qstrValue;

    if (qstrValue.length()) {
        QRectF rect = trf;

        int pos = 0;
        int idx;
        QChar separator;
        QRegExp re("\\s");
        QPrinter prnt(QPrinter::HighResolution);
        QFontMetrics fm(font(), &prnt);

        // int   intRectWidth    = (int)(trf.width() * prnt.resolution()) - 10;
        int   intRectWidth    = (int)((m_size.toPoint().width() / 72) * prnt.resolution());

        while (qstrValue.length()) {
            idx = re.indexIn(qstrValue, pos);
            if (idx == -1) {
                idx = qstrValue.length();
                separator = QChar('\n');
            } else
                separator = qstrValue.at(idx);

            if (fm.boundingRect(qstrValue.left(idx)).width() < intRectWidth || pos == 0) {
                pos = idx + 1;
                if (separator == '\n') {
                    QString line = qstrValue.left(idx);
                    qstrValue = qstrValue.mid(idx + 1, qstrValue.length());
                    pos = 0;

                    rect.setTop(intBaseTop + (intLineCounter * intRectHeight));
                    rect.setBottom(rect.top() + intRectHeight);

                    OROTextBox * tb = new OROTextBox();
                    tb->setPosition(rect.topLeft());
                    tb->setSize(rect.size());
                    tb->setFont(font());
                    tb->setText(line);
                    tb->setFlags(textFlags());
                    tb->setTextStyle(textStyle());
                    tb->setLineStyle(lineStyle());
                    
                    if (page) {
                        page->addPrimitive(tb);
                    }
                    
                    if (section) {
                        OROTextBox *tb2 = dynamic_cast<OROTextBox*>(tb->clone());
                        tb2->setPosition(m_pos.toPoint());
                        section->addPrimitive(tb2);
                    }
                    
                    if (!page) {
                        delete tb;
                    }
    
                    intStretch += intRectHeight;
                    intLineCounter++;
                }
            } else {
                QString line = qstrValue.left(pos - 1);
                qstrValue = qstrValue.mid(pos, qstrValue.length());
                pos = 0;

                rect.setTop(intBaseTop + (intLineCounter * intRectHeight));
                rect.setBottom(rect.top() + intRectHeight);

                OROTextBox * tb = new OROTextBox();
                tb->setPosition(rect.topLeft());
                tb->setSize(rect.size());
                tb->setFont(font());
                tb->setText(line);
                tb->setFlags(textFlags());
                tb->setTextStyle(textStyle());
                tb->setLineStyle(lineStyle());
                if (page) page->addPrimitive(tb);

                intStretch += intRectHeight;
                intLineCounter++;
            }
        }

        intStretch += (m_bottomPadding / 100.0);
    }
    
    return intStretch; //Item returns its required section height
}
