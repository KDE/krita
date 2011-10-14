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
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlNS.h>
#include <KoInlineTextObjectManager.h>
#include <KoVariableManager.h>
#include <KoTextDocument.h>

#include <QDateTime>
#include <QTextInlineObject>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <klocale.h>
#include <kdebug.h>

UserVariable::UserVariable()
    : QObject()
    , KoVariable(true)
    , m_variableManager(0)
    , m_property(0)
{
    connect(&m_nameMapper, SIGNAL(mapped(QWidget*)), this, SLOT(nameChanged(QWidget*)));
    connect(&m_typeMapper, SIGNAL(mapped(QWidget*)), this, SLOT(typeChanged(QWidget*)));
    connect(&m_valueMapper, SIGNAL(mapped(QWidget*)), this, SLOT(valueChanged(QWidget*)));
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
    QWidget *configWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(configWidget);
    layout->setColumnStretch(1, 1);
    configWidget->setLayout(layout);

    QLabel *nameLabel = new QLabel(i18n("Name:"), configWidget);
    nameLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(nameLabel, 0, 0);
    QHBoxLayout *nameLayout = new QHBoxLayout(configWidget);
    QComboBox *nameEdit = new QComboBox(configWidget);
    nameEdit->setObjectName(QLatin1String("nameEdit"));
    nameLabel->setBuddy(nameEdit);
    QStringList names = variableManager()->userVariables();
    nameEdit->addItems(names);
    if (m_name.isNull() && !names.isEmpty()) {
        m_name = names.first();
    }
    nameEdit->setCurrentIndex(qMax(0, names.indexOf(m_name)));
    connect(nameEdit, SIGNAL(currentIndexChanged(QString)), &m_nameMapper, SLOT(map()));
    m_nameMapper.setMapping(nameEdit, configWidget);
    nameLayout->addWidget(nameEdit);

    QPushButton *newButton = new QPushButton(i18n("New"), configWidget);
    connect(newButton, SIGNAL(clicked()), this, SLOT(newClicked()));
    nameLayout->addWidget(newButton);

    QPushButton *deleteButton = new QPushButton(i18n("Delete"), configWidget);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteClicked()));
    nameLayout->addWidget(deleteButton);

    layout->addLayout(nameLayout, 0, 1);

    QLabel *typeLabel = new QLabel(i18n("Format:"), configWidget);
    typeLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(typeLabel, 1, 0);
    QComboBox *typeEdit = new QComboBox(configWidget);
    typeEdit->setObjectName(QLatin1String("typeEdit"));
    typeLabel->setBuddy(typeEdit);
    typeEdit->addItem(i18n("String"), QLatin1String("string"));
    typeEdit->addItem(i18n("Boolean"), QLatin1String("boolean"));
    typeEdit->addItem(i18n("Float"), QLatin1String("float"));
    typeEdit->addItem(i18n("Percentage"), QLatin1String("percentage"));
    typeEdit->addItem(i18n("Currency"), QLatin1String("currency"));
    typeEdit->addItem(i18n("Date"), QLatin1String("date"));
    typeEdit->addItem(i18n("Time"), QLatin1String("time"));
    typeEdit->addItem(i18n("Void"), QLatin1String("void"));
    typeEdit->addItem(i18n("Formula"), QLatin1String("formula"));
    typeEdit->setCurrentIndex(qMax(0, typeEdit->findData(variableManager()->userType(m_name))));
    connect(typeEdit, SIGNAL(currentIndexChanged(QString)), &m_typeMapper, SLOT(map()));
    m_typeMapper.setMapping(typeEdit, configWidget);
    layout->addWidget(typeEdit, 1, 1);

    QLabel *valueLabel = new QLabel(i18n("Value:"), configWidget);
    valueLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(valueLabel, 2, 0);
    QLineEdit *valueEdit = new QLineEdit(configWidget);
    valueEdit->setObjectName(QLatin1String("valueEdit"));
    valueLabel->setBuddy(valueEdit);
    valueEdit->setText(variableManager()->value(m_name));
    connect(valueEdit, SIGNAL(textChanged(QString)), &m_valueMapper, SLOT(map()));
    m_valueMapper.setMapping(valueEdit, configWidget);
    layout->addWidget(valueEdit, 2, 1);
    valueChanged();

    return configWidget;
}

QComboBox* UserVariable::nameEdit(QWidget *configWidget) const
{
    QComboBox *c = configWidget->findChild<QComboBox*>("nameEdit");
    Q_ASSERT(c);
    return c;
}

QComboBox* UserVariable::typeEdit(QWidget *configWidget) const
{
    QComboBox *c = configWidget->findChild<QComboBox*>("typeEdit");
    Q_ASSERT(c);
    return c;
}

QLineEdit* UserVariable::valueEdit(QWidget *configWidget) const
{
    QLineEdit *e = configWidget->findChild<QLineEdit*>("valueEdit");
    Q_ASSERT(e);
    return e;
}

void UserVariable::nameChanged(QWidget *configWidget)
{
    m_name = nameEdit(configWidget)->currentText();

    QLineEdit *ve = valueEdit(configWidget);
    bool wasBlocked = ve->blockSignals(true);
    ve->setText(variableManager()->value(m_name));
    ve->blockSignals(wasBlocked);

    valueChanged();
}

void UserVariable::typeChanged(QWidget *configWidget)
{
    QString value = variableManager()->value(m_name);

    QComboBox *typeedit = typeEdit(configWidget);
    QString type = typeedit->itemData(typeedit->currentIndex()).toString();

    variableManager()->setValue(m_name, value, type);

    //valueChanged();
}

void UserVariable::valueChanged(QWidget *configWidget)
{
    QString value = valueEdit(configWidget)->text();
    QString type = variableManager()->userType(m_name);
    variableManager()->setValue(m_name, value, type);
    //valueChanged();
}

void UserVariable::newClicked()
{
}

void UserVariable::deleteClicked()
{
}

void UserVariable::valueChanged()
{
    QString value = variableManager()->value(m_name);

    /* TODO seems we do not support custom formats at all :-/
       see also the "TODO see 6.2.1 Date Fields" at plugins/variables/DateVariable.cpp:96

    switch (m_numberstyle.type) {
        case KoOdfNumberStyles::Date:
        case KoOdfNumberStyles::Time: {
            //QDateTime dt = QDateTime::fromTime_t(value.toUInt())
            value = dt.toString(m_numberstyle.prefix + m_numberstyle.formatStr + m_numberstyle.suffix);
        } break;
        case KoOdfNumberStyles::Scientific:
        case KoOdfNumberStyles::Fraction:
        case KoOdfNumberStyles::Currency:
        case KoOdfNumberStyles::Percentage:
        case KoOdfNumberStyles::Boolean:
        case KoOdfNumberStyles::Number:
        case KoOdfNumberStyles::Text:
            value = m_numberstyle.prefix + value + m_numberstyle.suffix;
            break;
    }
    */

    //kDebug() << m_name << value;
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
    KoXmlWriter *writer = &context.xmlWriter();
    switch (m_property) {
        case KoInlineObject::UserGet: {
            writer->startElement("text:user-field-get", false);
            if (!m_name.isEmpty())
                writer->addAttribute("text:name", m_name);
            writer->addTextNode(value());
            writer->endElement();
        } break;
        case KoInlineObject::UserInput: {
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
