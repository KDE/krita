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
#include <KoVariable.h>

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
    void nameChanged(const QString &name);
    void valueChanged();

private:
    KoVariableManager *m_variableManager;
    KoVariableManager *variableManager();

    void resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);

    int m_type;
    QString m_name;
    
};

#endif
