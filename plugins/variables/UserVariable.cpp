/* This file is part of the KDE project
 * Copyright (C) 2011 Sebastian Sauer <mail@dipe.org>
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

#include "UserVariable.h"
#include "UserVariableOptionsWidget.h"

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoProperties.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlNS.h>
#include <KoInlineTextObjectManager.h>
#include <KoVariableManager.h>
#include <KoTextDocument.h>

#include <QTextInlineObject>
#include <KLocale>
#include <KDebug>

UserVariable::UserVariable()
    : KoVariable(true)
    , m_variableManager(0)
    , m_property(0)
{
}

KoVariableManager *UserVariable::variableManager()
{
    if (m_variableManager) {
        return m_variableManager;
    }

    KoInlineTextObjectManager *textObjectManager = manager();
    Q_ASSERT(textObjectManager);
    m_variableManager = textObjectManager->variableManager();
    Q_ASSERT(m_variableManager);
    connect(m_variableManager, SIGNAL(valueChanged()), this, SLOT(valueChanged()));

    valueChanged(); // initial update

    return m_variableManager;
}

int UserVariable::property() const
{
    return m_property;
}

const QString& UserVariable::name() const
{
    return m_name;
}

void UserVariable::setName(const QString &name)
{
    m_name = name;
    valueChanged();
}

KoOdfNumberStyles::NumericStyleFormat UserVariable::numberstyle() const
{
    return m_numberstyle;
}

void UserVariable::setNumberStyle(KoOdfNumberStyles::NumericStyleFormat numberstyle)
{
    m_numberstyle = numberstyle;
    valueChanged();
}

QWidget* UserVariable::createOptionsWidget()
{
    UserVariableOptionsWidget *configWidget = new UserVariableOptionsWidget(this);
    return configWidget;
}

void UserVariable::valueChanged()
{
    //TODO apply following also to plugins/variables/DateVariable.cpp:96
    //TODO handle formula
    QString value = variableManager()->value(m_name);
    value = KoOdfNumberStyles::format(value, m_numberstyle);
    setValue(value);
}

void UserVariable::readProperties(const KoProperties *props)
{
    m_property = props->intProperty("varproperty");
    //m_name = props->stringProperty("varname");
    //kDebug() << m_property << m_name;
    //valueChanged();
}

void UserVariable::propertyChanged(Property property, const QVariant &value)
{
    Q_UNUSED(property);
    Q_UNUSED(value);
    //setValue(value.toString());
}

void UserVariable::saveOdf(KoShapeSavingContext &context)
{
    if (m_property == 0 && !variableManager()->userVariables().contains(m_name))
        return;

    KoXmlWriter *writer = &context.xmlWriter();

    if (m_property == KoInlineObject::UserGet)
        writer->startElement("text:user-field-get", false);
    else
        writer->startElement("text:user-field-input", false);

    if (!m_name.isEmpty())
        writer->addAttribute("text:name", m_name);

    QString styleName = KoOdfNumberStyles::saveOdfNumberStyle(context.mainStyles(), m_numberstyle);
    if (!styleName.isEmpty())
        writer->addAttribute("style:data-style-name", styleName);

    writer->addTextNode(value());
    writer->endElement();
}

bool UserVariable::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    if (element.localName() == "user-field-get") {
        m_property = KoInlineObject::UserGet;
    } else if (element.localName() == "user-field-input") {
        m_property = KoInlineObject::UserInput;
    } else {
        m_property = 0;
    }

    m_name = element.attributeNS(KoXmlNS::text, "name");

    QString dataStyle = element.attributeNS(KoXmlNS::style, "data-style-name");
    if (!dataStyle.isEmpty() && context.odfLoadingContext().stylesReader().dataFormats().contains(dataStyle)) {
        m_numberstyle = context.odfLoadingContext().stylesReader().dataFormats().value(dataStyle).first;
    } else {
        m_numberstyle = KoOdfNumberStyles::NumericStyleFormat();
    }

    return true;
}

void UserVariable::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    KoVariable::resize(document, object, posInDocument, format, pd);

    if (!m_variableManager) {
        variableManager();
    }
}
