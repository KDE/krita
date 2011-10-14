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

#ifndef PAGEVARIABLE_H
#define PAGEVARIABLE_H

#include <QObject>
#include <QSignalMapper>
#include <KoVariable.h>
#include <KoOdfNumberStyles.h>

class QLineEdit;
class QComboBox;
class KoShapeSavingContext;
class KoVariableManager;

/**
 * This is a KoVariable for user defined variables.
 *
 * This implements the ODF attributes text:user-field-get and
 * text:user-field-input which are fetching variables defined
 * via text:user-field-decls and text:user-field-decl.
 */
class UserVariable : public QObject, public KoVariable
{
    Q_OBJECT
public:
    UserVariable();

    QWidget* createOptionsWidget();
    void readProperties(const KoProperties *props);
    void propertyChanged(Property property, const QVariant &value);
    void saveOdf(KoShapeSavingContext &context);
    bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext & context);

private Q_SLOTS:
    void nameChanged(QWidget *configWidget);
    void typeChanged(QWidget *configWidget);
    void valueChanged(QWidget *configWidget);
    void newClicked();
    void deleteClicked();
    void valueChanged();

private:
    KoVariableManager *variableManager();

    void resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);

    QComboBox* nameEdit(QWidget *configWidget) const;
    QComboBox* typeEdit(QWidget *configWidget) const;
    QLineEdit* valueEdit(QWidget *configWidget) const;

    KoVariableManager *m_variableManager;
    int m_property;
    QString m_name;

    KoOdfNumberStyles::NumericStyleFormat m_numberstyle;
    QSignalMapper m_nameMapper, m_typeMapper, m_valueMapper;
};

#endif
