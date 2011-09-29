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

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoProperties.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlNS.h>
#include <KoInlineTextObjectManager.h>
#include <KoVariableManager.h>
#include <KoTextDocument.h>

#include <QTextInlineObject>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <klocale.h>
#include <kdebug.h>

UserVariable::UserVariable()
    : QObject()
    , KoVariable(true)
    , m_type(0)
    , m_variableManager(0)
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

QWidget* UserVariable::createOptionsWidget()
{
    QWidget *widget = new QWidget();
    QGridLayout *layout = new QGridLayout(widget);
    layout->setColumnStretch(1, 1);
    widget->setLayout(layout);

    QLabel *nameLabel = new QLabel(i18n("Name:"), widget);
    nameLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(nameLabel, 0, 0);
    QComboBox *nameEdit = new QComboBox(widget);
    nameLabel->setBuddy(nameEdit);
    nameEdit->addItems(variableManager()->userVariables());
    nameEdit->setCurrentIndex(2);
    layout->addWidget(nameEdit, 0, 1);
    connect(nameEdit, SIGNAL(currentIndexChanged(QString)), this, SLOT(nameChanged(QString)));

    return widget;
}

void UserVariable::nameChanged(const QString &name)
{
    kDebug() << name;
    m_name = name;
    valueChanged();
}

void UserVariable::valueChanged()
{
    Q_ASSERT(m_variableManager);
    QString value = m_variableManager->value(m_name);
    kDebug() << m_name << value;
    setValue(value);
}

void UserVariable::readProperties(const KoProperties *props)
{
    m_type = props->intProperty("vartype");
    //m_name = props->stringProperty("varname");
    //kDebug() << m_type << m_name;
    //valueChanged();
}

void UserVariable::propertyChanged(Property property, const QVariant &value)
{
    //setValue(value.toString());
}

void UserVariable::saveOdf(KoShapeSavingContext &context)
{
    KoXmlWriter *writer = &context.xmlWriter();
    switch (m_type) {
        case 1: {
            writer->startElement("text:user-field-get", false);
            if (!m_name.isEmpty())
                writer->addAttribute("text:name", m_name);
            writer->addTextNode(value());
            writer->endElement();
        } break;
        case 2: {
            writer->startElement("text:user-field-input", false);
            if (!m_name.isEmpty())
                writer->addAttribute("text:name", m_name);
            writer->addTextNode(value());
            writer->endElement();
        } break;
        default:
            break;
    }
}

bool UserVariable::loadOdf(const KoXmlElement &element, KoShapeLoadingContext&)
{
    if (element.localName() == "user-field-get") {
        m_type = 1;
    } else if (element.localName() == "user-field-input") {
        m_type = 2;
    } else {
        m_type = 0;
    }

    m_name = element.attributeNS(KoXmlNS::text, "name");

    return true;
}

void UserVariable::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    if (!m_variableManager) {
        variableManager();
    }
}
