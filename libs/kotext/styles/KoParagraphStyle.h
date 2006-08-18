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
class QTextBlock;

/**
 * A container for all properties for the paragraph wide style.
 * Each paragraph in the main text either is based on a parag style, or its not. Where
 * it is based on a paragraph style this is indecated that it has a property 'StyleId'
 * with an integer as value.  The integer value corresponds to the styleId() output of
 * a specific KoParagraphStyle.
 */
class KOTEXT_EXPORT KoParagraphStyle : public QObject {
    Q_OBJECT
public:
    enum Property {
        StyleId = QTextFormat::UserProperty+1,
        // Linespacing properties
        FixedLineHeight,    ///< this propery is used to use a non-default line height
        MinimumLineHeight,  ///< this property is used to have a minimum line spacing
        LineSpacing,        ///< Hard leader height.
        FontIndependentLineSpacing  ///< if true, use fontsize (in pt) solely.
    };

    KoParagraphStyle();
    ~KoParagraphStyle();

    //  ***** Linespacing stuff from ODF ****
    /**
     * Sets the line height as a percentage of the highest character on that line.
     * A good typographically correct value would be 120%
     * @see setFontIndependentLineSpacing
     */
    void setLineHeightPercent(int lineHeight)
        {setProperty(FixedLineHeight, lineHeight); remove(LineSpacing);}
    int lineHeightPercent() { return propertyInt(FixedLineHeight); }

    /**
     * Sets the line height to a specific pt-based height, ignoring the font size.
     */
    void setLineHeightAbsolute(double height)
        {setProperty(FixedLineHeight, height); remove(LineSpacing);}
    double lineHeightAbsolute() {return propertyDouble(FixedLineHeight); }

    /**
     * Sets the line height to have a minimum height in pt.
     */
    void setMinimumLineHeight(double height) {setProperty(MinimumLineHeight, height); }
    double minimumLineHeight() { return propertyDouble(MinimumLineHeight); }

    /**
     * Sets the space between two lines to be a specific height, ignoring the font size.
     */
    void setLineSpacing(double spacing)
        {setProperty(LineSpacing, spacing); remove(FixedLineHeight); }
    double lineSpacing() {return propertyDouble(LineSpacing); }

    /**
     * If set to true the font-size will be used instead of the font-encoded size.
     * This property influences setLineHeightPercent() behavior.
     * When on (default) a font of 12pt will always have a linespacing of 12pt times the
     * current linespacing percentage.   When off the linespacing embedded in the font
     * is used which can differ for various fonts, even if they are the same size.
     */
    void setFontIndependentLineSpacing(bool on) {setProperty(FontIndependentLineSpacing, on); }
    bool fontIndependentLineSpacing() {return propertyBoolean(FontIndependentLineSpacing); }


    // properties from QTextFormat
    /// duplicated property from QTextBlockFormat
    void setTopMargin(double topMargin) { setProperty(QTextFormat::BlockTopMargin, topMargin); }
    /// duplicated property from QTextBlockFormat
    double topMargin() const { return propertyDouble(QTextFormat::BlockTopMargin); }

    /// duplicated property from QTextBlockFormat
    void setBottomMargin (double margin) { setProperty(QTextFormat::BlockBottomMargin, margin); }
    /// duplicated property from QTextBlockFormat
    double bottomMargin () const { return propertyDouble(QTextFormat::BlockBottomMargin); }
    /// duplicated property from QTextBlockFormat
    void setLeftMargin (double margin) { setProperty(QTextFormat::BlockLeftMargin, margin); }
    /// duplicated property from QTextBlockFormat
    double leftMargin () const { return propertyDouble(QTextFormat::BlockLeftMargin); }
    /// duplicated property from QTextBlockFormat
    void setRightMargin (double margin) { setProperty(QTextFormat::BlockRightMargin, margin); }
    /// duplicated property from QTextBlockFormat
    double rightMargin () const { return propertyDouble(QTextFormat::BlockRightMargin); }

    /// duplicated property from QTextBlockFormat
    void setAlignment (Qt::Alignment alignment) {
        setProperty(QTextFormat::BlockAlignment, (int) alignment);
    }
    /// duplicated property from QTextBlockFormat
    Qt::Alignment alignment () const {
        return static_cast<Qt::Alignment> (propertyInt(QTextFormat::BlockAlignment));
    }
    /// duplicated property from QTextBlockFormat
    void setTextIndent (double margin) { setProperty(QTextFormat::TextIndent, margin); }
    /// duplicated property from QTextBlockFormat
    double textIndent () const { return propertyDouble(QTextFormat::TextIndent); }
    /// duplicated property from QTextBlockFormat
    void setIndent (int indent) { setProperty(QTextFormat::BlockIndent, indent); }
    /// duplicated property from QTextBlockFormat
    int indent () const { return propertyInt(QTextFormat::BlockIndent); }

    /// set the parent style this one inherits its unset properties from.
    void setParent(KoParagraphStyle *parent);

    /// return the parent style
    KoParagraphStyle *parent() const { return m_parent; }

    /// the 'next' style is the one used when the user creates a new paragrap after this one.
    void setNextStyle(int next) { m_next = next; }

    /// the 'next' style is the one used when the user creates a new paragrap after this one.
    int nextStyle() const { return m_next; }

    /// return the name of the style.
    const QString& name() const { return m_name; }

    /// set a user-visible name on the style.
    void setName(const QString &name) { m_name = name; }

    /// each style has a unique ID (non persistant) given out by the styleManager
    int styleId() const { return propertyInt(StyleId); }

    /// each style has a unique ID (non persistant) given out by the styleManager
    void setStyleId(int id) { setProperty(StyleId, id); if(m_next == 0) m_next=id; }

    /**
     * Apply this style to a blockFormat by copying all properties from this, and parent
     * styles to the target block format.  Note that the character format will not be applied
     * using this method, use the other applyStyle() method for that.
     */
    void applyStyle(QTextBlockFormat &format) const;

    /**
     * Apply this style to the textBlock by copying all properties from this, parent and
     * the character style (where relevant) to the target block formats.
     */
    void applyStyle(QTextBlock &block) const;

private:
    void setProperty(int key, const QVariant &value);
    void remove(int key);
    double propertyDouble(int key) const;
    int propertyInt(int key) const;
    bool propertyBoolean(int key) const;
    QVariant const *get(int key) const;

private:
    QString m_name;
    KoCharacterStyle *m_charStyle;
    KoParagraphStyle *m_parent;
    int m_next;
    StylePrivate *m_stylesPrivate;
};

#endif
