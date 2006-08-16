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
#ifndef KOCHARACTERSTYLE_H
#define KOCHARACTERSTYLE_H

#include <QObject>
#include <QVector>
#include <QVariant>
#include <QString>
#include <QTextCharFormat>
#include <koffice_export.h>

class StylePrivate;
class QTextBlock;

class KOTEXT_EXPORT KoCharacterStyle : public QObject {
    Q_OBJECT
public:
    enum Property {
        StyleId = QTextFormat::UserProperty+1
    };

    KoCharacterStyle();
    ~KoCharacterStyle();
/*
    void setFont (const QFont &font) { setProperty(QTextFormat::FOO, font); }
    QFont font () const {
        return static_cast<QFont> (propertyObject(QTextFormat::FOO));
    }
    void setFontFamily (const QString &family) { setProperty(QTextFormat::FOO, family); }
    QString fontFamily () const { return propertyString(QTextFormat::FOO); }
*/
    void setFontPointSize (qreal size) { setProperty(QTextFormat::FontPointSize, size); }
    double fontPointSize () const { return propertyDouble(QTextFormat::FontPointSize); }
    void setFontWeight (int weight) { setProperty(QTextFormat::FontWeight, weight); }
    int fontWeight () const { return propertyInt(QTextFormat::FontWeight); }
    void setFontItalic (bool italic) { setProperty(QTextFormat::FontItalic, italic); }
    bool fontItalic () const { return propertyBoolean(QTextFormat::FontItalic); }
    void setFontOverline (bool overline) { setProperty(QTextFormat::FontOverline, overline); }
    bool fontOverline () const { return propertyBoolean(QTextFormat::FontOverline); }
    void setFontStrikeOut (bool strikeOut) { setProperty(QTextFormat::FontStrikeOut, strikeOut); }
    bool fontStrikeOut () const { return propertyBoolean(QTextFormat::FontStrikeOut); }
    void setUnderlineColor (const QColor &color) { setProperty(QTextFormat::TextUnderlineColor, color); }
    QColor underlineColor () const;
    void setFontFixedPitch (bool fixedPitch) { setProperty(QTextFormat::FontFixedPitch, fixedPitch); }
    bool fontFixedPitch () const { return propertyBoolean(QTextFormat::FontFixedPitch); }
    void setUnderlineStyle (QTextCharFormat::UnderlineStyle style) {
        setProperty(QTextFormat::TextUnderlineStyle, style);
    }
    QTextCharFormat::UnderlineStyle underlineStyle () const {
        return static_cast<QTextCharFormat::UnderlineStyle> (propertyInt(QTextFormat::TextUnderlineStyle));
    }
    void setVerticalAlignment (QTextCharFormat::VerticalAlignment alignment) {
        setProperty(QTextFormat::TextVerticalAlignment, alignment);
    }
    QTextCharFormat::VerticalAlignment verticalAlignment () const {
        return static_cast<QTextCharFormat::VerticalAlignment> (propertyInt(QTextFormat::TextVerticalAlignment));
    }
    void setTextOutline (const QPen &pen) { setProperty(QTextFormat::TextOutline, pen); }
    QPen textOutline () const;


    const QString& name() const { return m_name; }

    void setName(const QString &name) { m_name = name; }

    int styleId() const { return propertyInt(StyleId); }

    void setStyleId(int id) { setProperty(StyleId, id); }

    void applyStyle(QTextCharFormat &format) const;
    void applyStyle(QTextBlock &block) const;

private:
    void setProperty(int key, const QVariant &value);
    const QVariant *get(int key) const;
    double propertyDouble(int key) const;
    int propertyInt(int key) const;
    const QString &propertyString(int key) const;
    bool propertyBoolean(int key) const;

private:
    QString m_name;
    StylePrivate *m_stylesPrivate;
};

#endif
