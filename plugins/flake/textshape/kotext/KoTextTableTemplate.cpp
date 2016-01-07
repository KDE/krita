/* This file is part of the KDE project
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoTextTableTemplate.h"

#include "KoTextSharedLoadingData.h"

#include <KoShapeLoadingContext.h>
#include <KoTextSharedSavingData.h>
#include <KoOdfWorkaround.h>
#include <KoXmlNS.h>
#include <KoTableCellStyle.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>


#include "Styles_p.h"

#include "TextDebug.h"

static const struct {
    KoTextTableTemplate::Property m_property;
    const char *m_element;
} templateStyles[] = {
    { KoTextTableTemplate::BackGround,    "background"   },
    { KoTextTableTemplate::Body,          "body"         },
    { KoTextTableTemplate::EvenColumns,   "even-columns" },
    { KoTextTableTemplate::EvenRows,      "even-rows"    },
    { KoTextTableTemplate::FirstColumn,   "first-column" },
    { KoTextTableTemplate::FirstRow,      "first-row"    },
    { KoTextTableTemplate::LastColumn,    "last-column"  },
    { KoTextTableTemplate::LastRow,       "last-row"     },
    { KoTextTableTemplate::OddColumns,    "odd-columns"  },
    { KoTextTableTemplate::OddRows,       "odd-rows"     }
};

static const unsigned int numTemplateStyles = sizeof(templateStyles) / sizeof(*templateStyles);

class Q_DECL_HIDDEN KoTextTableTemplate::Private
{
public:
    Private() { }

    ~Private() { }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }
    int propertyInt(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return 0;
        return variant.toInt();
    }

    StylePrivate stylesPrivate;
    QString name;
};


KoTextTableTemplate::KoTextTableTemplate(QObject *parent)
    : QObject(parent),
      d(new Private())
{

}

KoTextTableTemplate::~KoTextTableTemplate()
{
    delete d;
}

QString KoTextTableTemplate::name() const
{
    return d->name;
}

void KoTextTableTemplate::setName(const QString &name)
{
    if (name == d->name)
        return;
    d->name = name;
}

void KoTextTableTemplate::setStyleId(int id)
{
    d->stylesPrivate.add(KoTextTableTemplate::StyleId, id);
}

int KoTextTableTemplate::styleId() const
{
    return d->propertyInt(StyleId);
}

int KoTextTableTemplate::background() const
{
    return d->propertyInt(BackGround);
}

void KoTextTableTemplate::setBackground(int styleId)
{
    d->stylesPrivate.add(BackGround, styleId);
}

int KoTextTableTemplate::body() const
{
    return d->propertyInt(Body);
}

void KoTextTableTemplate::setBody(int styleId)
{
    d->stylesPrivate.add(Body, styleId);
}

int KoTextTableTemplate::evenColumns() const
{
    return d->propertyInt(EvenColumns);
}

void KoTextTableTemplate::setEvenColumns(int styleId)
{
    d->stylesPrivate.add(EvenColumns, styleId);
}

int KoTextTableTemplate::evenRows() const
{
    return d->propertyInt(EvenRows);
}

void KoTextTableTemplate::setEvenRows(int styleId)
{
    d->stylesPrivate.add(EvenRows, styleId);
}

int KoTextTableTemplate::firstColumn() const
{
    return d->propertyInt(FirstColumn);
}

void KoTextTableTemplate::setFirstColumn(int styleId)
{
    d->stylesPrivate.add(FirstColumn, styleId);
}

int KoTextTableTemplate::firstRow() const
{
    return d->propertyInt(FirstRow);
}

void KoTextTableTemplate::setFirstRow(int styleId)
{
    d->stylesPrivate.add(FirstRow, styleId);
}

int KoTextTableTemplate::lastColumn() const
{
    return d->propertyInt(LastColumn);
}

void KoTextTableTemplate::setLastColumn(int styleId)
{
    d->stylesPrivate.add(LastColumn, styleId);
}

int KoTextTableTemplate::lastRow() const
{
    return d->propertyInt(LastRow);
}

void KoTextTableTemplate::setLastRow(int styleId)
{
    d->stylesPrivate.add(LastRow, styleId);
}

int KoTextTableTemplate::oddColumns() const
{
    return d->propertyInt(OddColumns);
}

void KoTextTableTemplate::setOddColumns(int styleId)
{
    d->stylesPrivate.add(OddColumns, styleId);
}

int KoTextTableTemplate::oddRows() const
{
    return d->propertyInt(OddRows);
}

void KoTextTableTemplate::setOddRows(int styleId)
{
    d->stylesPrivate.add(OddRows, styleId);
}

void KoTextTableTemplate::loadOdf(const KoXmlElement *element, KoShapeLoadingContext &context)
{
    QString templateName = element->attributeNS(KoXmlNS::table, "name", QString());
#ifndef NWORKAROUND_ODF_BUGS
    if (templateName.isEmpty()) {
        templateName = KoOdfWorkaround::fixTableTemplateName(*element);
    }
#endif
    d->name = templateName;

    KoSharedLoadingData *sharedData = context.sharedData(KOTEXT_SHARED_LOADING_ID);
    KoTextSharedLoadingData *textSharedData = 0;
    if (sharedData) {
        textSharedData = dynamic_cast<KoTextSharedLoadingData *>(sharedData);
    }

    if (textSharedData) {
        KoXmlElement styleElem;
        forEachElement(styleElem, (*element)) {
            if (styleElem.namespaceURI() == KoXmlNS::table) {
                for (uint index = 0; index < numTemplateStyles; ++index) {
                    if (templateStyles[index].m_element == styleElem.localName()) {
                        QString styleName = styleElem.attributeNS(KoXmlNS::table, "style-name", QString());
#ifndef NWORKAROUND_ODF_BUGS
                        if (styleName.isEmpty()) {
                            styleName = KoOdfWorkaround::fixTableTemplateCellStyleName(styleElem);
                        }
#endif
                        KoTableCellStyle *cs = 0;
                        if (!styleName.isEmpty()) {
                            cs = textSharedData->tableCellStyle(styleName, true);
                            if (!cs) {
                                warnText << "Missing KoTableCellStyle!";
                            }
                            else {
                                //                debugText << "==> cs.name:" << cs->name();
                                //                debugText << "==> cs.styleId:" << cs->styleId();
                                d->stylesPrivate.add(templateStyles[index].m_property, cs->styleId());
                            }
                        }
                    }
                }
            }
        }
    }
}

void KoTextTableTemplate::saveOdf(KoXmlWriter *writer, KoTextSharedSavingData *savingData) const
{
    writer->startElement("table:table-template");

    QString styleName(QString(QUrl::toPercentEncoding(name(), "", " ")).replace('%', '_'));
    if (styleName.isEmpty())
        styleName = "TT";

    QString generatedName = styleName;
    int num = 1;
    while (savingData->styleNames().contains(generatedName)) {
        generatedName = styleName + QString::number(num++);
    }

    savingData->setStyleName(styleId(), generatedName);
    d->name = generatedName;

    writer->addAttribute("table:name", name());

    for (uint index = 0; index < numTemplateStyles; ++index) {
        if (d->stylesPrivate.contains(templateStyles[index].m_property)) {
            writer->startElement(QString("table:%1").arg(templateStyles[index].m_element).toLatin1());
            QString savedStyleName = savingData->styleName(d->stylesPrivate.value(templateStyles[index].m_property).toInt());
            if (! savedStyleName.isEmpty()) {
                writer->addAttribute("table:style-name", savedStyleName);
            }

            writer->endElement();
        }
    }

    writer->endElement(); //table:table-template
}
