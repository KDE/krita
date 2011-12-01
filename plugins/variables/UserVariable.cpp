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

#include <QTextInlineObject>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QValidator>
#include <KLocale>
#include <KMessageBox>
#include <KInputDialog>
#include <KDebug>

UserVariableOptionsWidget::UserVariableOptionsWidget(UserVariable* userVariable, QWidget *parent)
    : QWidget(parent)
    , userVariable(userVariable)
{
    QGridLayout *layout = new QGridLayout(this);
    layout->setColumnStretch(1, 1);
    setLayout(layout);

    QLabel *nameLabel = new QLabel(i18n("Name:"), this);
    nameLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(nameLabel, 0, 0);
    QHBoxLayout *nameLayout = new QHBoxLayout(this);
    nameEdit = new QComboBox(this);
    nameEdit->setObjectName(QLatin1String("nameEdit"));
    nameEdit->setMinimumContentsLength(10);
    nameLabel->setBuddy(nameEdit);
    connect(nameEdit, SIGNAL(currentIndexChanged(QString)), this, SLOT(nameChanged()));
    nameLayout->addWidget(nameEdit);

    newButton = new QPushButton(i18n("New"), this);
    connect(newButton, SIGNAL(clicked()), this, SLOT(newClicked()));
    nameLayout->addWidget(newButton);

    deleteButton = new QPushButton(i18n("Delete"), this);
    deleteButton->setObjectName("DeleteButton");
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteClicked()));
    nameLayout->addWidget(deleteButton);

    layout->addLayout(nameLayout, 0, 1);

    QLabel *typeLabel = new QLabel(i18n("Format:"), this);
    typeLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(typeLabel, 1, 0);
    typeEdit = new QComboBox(this);
    typeEdit->setObjectName(QLatin1String("typeEdit"));
    typeLabel->setBuddy(typeEdit);
    typeEdit->addItem(i18n("String"), QLatin1String("string"));
    typeEdit->addItem(i18n("Boolean"), QLatin1String("boolean"));
    typeEdit->addItem(i18n("Float"), QLatin1String("float"));
    typeEdit->addItem(i18n("Percentage"), QLatin1String("percentage"));
    typeEdit->addItem(i18n("Currency"), QLatin1String("currency"));
    typeEdit->addItem(i18n("Date"), QLatin1String("date"));
    typeEdit->addItem(i18n("Time"), QLatin1String("time"));
    typeEdit->addItem(i18n("Formula"), QLatin1String("formula"));
    typeEdit->addItem(i18n("Void"), QLatin1String("void"));
    typeEdit->setCurrentIndex(qMax(0, typeEdit->findData(variableManager()->userType(userVariable->name()))));
    connect(typeEdit, SIGNAL(currentIndexChanged(QString)), this, SLOT(typeChanged()));
    layout->addWidget(typeEdit, 1, 1);

    QLabel *valueLabel = new QLabel(i18n("Value:"), this);
    valueLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(valueLabel, 2, 0);
    valueEdit = new QLineEdit(this);
    valueEdit->setObjectName(QLatin1String("valueEdit"));
    valueLabel->setBuddy(valueEdit);
    valueEdit->setText(variableManager()->value(userVariable->name()));
    connect(valueEdit, SIGNAL(textChanged(QString)), this, SLOT(valueChanged()));
    layout->addWidget(valueEdit, 2, 1);

    updateNameEdit();
}

UserVariableOptionsWidget::~UserVariableOptionsWidget()
{
}

KoVariableManager *UserVariableOptionsWidget::variableManager()
{
    return userVariable->variableManager();
}

void UserVariableOptionsWidget::nameChanged()
{
    bool enabled = !variableManager()->userVariables().isEmpty();

    nameEdit->setEnabled(enabled);
    userVariable->setName(nameEdit->currentText());

    bool wasBlocked = typeEdit->blockSignals(true);
    typeEdit->setCurrentIndex(qMax(0, typeEdit->findData(variableManager()->userType(userVariable->name()))));
    typeEdit->blockSignals(wasBlocked);
    typeEdit->setEnabled(enabled);

    wasBlocked = valueEdit->blockSignals(true);
    valueEdit->setText(variableManager()->value(userVariable->name()));
    valueEdit->blockSignals(wasBlocked);
    valueEdit->setEnabled(enabled);

    deleteButton->setEnabled(enabled);
}

void UserVariableOptionsWidget::typeChanged()
{
    QString value = variableManager()->value(userVariable->name());
    QString type = typeEdit->itemData(typeEdit->currentIndex()).toString();
    variableManager()->setValue(userVariable->name(), value, type);
    //userVariable->valueChanged();
}

void UserVariableOptionsWidget::valueChanged()
{
    QString value = valueEdit->text();
    QString type = variableManager()->userType(userVariable->name());
    variableManager()->setValue(userVariable->name(), value, type);
    //userVariable->valueChanged();
}

void UserVariableOptionsWidget::newClicked()
{
    class Validator : public QValidator
    {
    public:
        Validator(KoVariableManager *variableManager) : m_variableManager(variableManager) {}
        virtual State validate(QString &input, int &) const
        {
            QString s = input.trimmed();
            return s.isEmpty() || m_variableManager->userVariables().contains(s) ? Intermediate : Acceptable;
        }
    private:
        KoVariableManager *m_variableManager;
    };
    Validator validator(variableManager());
    QString name = KInputDialog::getText(i18n("New Variable"), i18n("Name for new variable:"), QString(), 0, this, &validator).trimmed();
    if (name.isEmpty()) {
        return;
    }
    userVariable->setName(name);
    variableManager()->setValue(userVariable->name(), QString(), QLatin1String("string"));
    updateNameEdit();
    valueEdit->setFocus();
}

void UserVariableOptionsWidget::deleteClicked()
{
    if (!variableManager()->userVariables().contains(userVariable->name())) {
        return;
    }
    if (KMessageBox::questionYesNo(this,
            i18n("Delete variable <b>%1</b>?", userVariable->name()),
            i18n("Delete Variable"),
            KStandardGuiItem::yes(),
            KStandardGuiItem::cancel(),
            QString(),
            KMessageBox::Dangerous | KMessageBox::Notify) != KMessageBox::Yes) {
        return;
    }
    variableManager()->remove(userVariable->name());
    userVariable->setName(QString());
    updateNameEdit();
}

void UserVariableOptionsWidget::updateNameEdit()
{
    QStringList names = variableManager()->userVariables();
    bool wasBlocked = nameEdit->blockSignals(true);
    nameEdit->clear();
    nameEdit->addItems(names);
    nameEdit->blockSignals(wasBlocked);
    if (userVariable->name().isNull() && !names.isEmpty()) {
        userVariable->setName(names.first());
    }
    nameEdit->setCurrentIndex(qMax(0, names.indexOf(userVariable->name())));
    nameChanged();
}

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
