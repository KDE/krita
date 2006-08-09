/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
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
#ifndef KOPARAGRAPHSTYLE_H
#define KOPARAGRAPHSTYLE_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QVariant>
#include <QTextFormat>
#include <koffice_export.h>

struct Property;
class KoCharacterStyle;
class StylePrivate;

class KOTEXT_EXPORT KoParagraphStyle : public QObject {
    Q_OBJECT
public:
    enum Property {
        StyleId = QTextFormat::UserProperty+1
    };

    KoParagraphStyle();
    ~KoParagraphStyle();

/*
    this class has setters and getters for all properties that we record in a style.
    The setter will set the property unless the m_parent has that exact same value
    in which case it will clear the property on m_paragStyle.
*/

    void setTopMargin(double topMargin) { setProperty(QTextFormat::BlockTopMargin, topMargin); }
    double topMargin() const { return propertyDouble(QTextFormat::BlockTopMargin); }

    void setBottomMargin (double margin) { setProperty(QTextFormat::BlockBottomMargin, margin); }
    double bottomMargin () const { return propertyDouble(QTextFormat::BlockBottomMargin); }
    void setLeftMargin (double margin) { setProperty(QTextFormat::BlockLeftMargin, margin); }
    double leftMargin () const { return propertyDouble(QTextFormat::BlockLeftMargin); }
    void setRightMargin (double margin) { setProperty(QTextFormat::BlockRightMargin, margin); }
    double rightMargin () const { return propertyDouble(QTextFormat::BlockRightMargin); }

    void setAlignment (Qt::Alignment alignment) {
        setProperty(QTextFormat::BlockAlignment, (int) alignment);
    }
    Qt::Alignment alignment () const {
        return static_cast<Qt::Alignment> (propertyInt(QTextFormat::BlockAlignment));
    }
    void setTextIndent (double margin) { setProperty(QTextFormat::TextIndent, margin); }
    double textIndent () const { return propertyDouble(QTextFormat::TextIndent); }
    void setIndent (int indent) { setProperty(QTextFormat::BlockIndent, indent); }
    int indent () const { return propertyInt(QTextFormat::BlockIndent); }

    void setParent(KoParagraphStyle *parent);

    KoParagraphStyle *parent() const { return m_parent; }

    void setNext(const KoParagraphStyle *next) { m_next = next; }

    const KoParagraphStyle *next() const { return m_next; }

    const QString& name() const { return m_name; }

    void setName(const QString &name) { m_name = name; }

    int styleId() const { return propertyInt(StyleId); }

    void setStyleId(int id) { setProperty(StyleId, id); }

    void applyStyle(QTextBlockFormat &format) const;

private:
    void setProperty(int key, const QVariant &value);
    double propertyDouble(int key) const;
    int propertyInt(int key) const;
    const QVariant *get(int key) const;

private:
    QString m_name;
    KoCharacterStyle *m_charStyle;
    KoParagraphStyle *m_parent;
    KoParagraphStyle const *m_next;
    StylePrivate *m_stylesPrivate;
};

#endif
