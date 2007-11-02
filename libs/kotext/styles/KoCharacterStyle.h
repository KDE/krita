/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#ifndef KOCHARACTERSTYLE_H
#define KOCHARACTERSTYLE_H

#include <QObject>
#include <QVector>
#include <QVariant>
#include <QString>
#include <QTextCharFormat>
#include <kotext_export.h>

class StylePrivate;
class QTextBlock;
class KoStyleStack;
class KoTextLoadingContext;
class KoGenStyle;

/**
 * A container for all properties for a character style.
 * A character style represents all character properties for a set of characters.
 * Each character in the document will have a character style, most of the time
 * shared with all the characters next to it that have the same style (see
 * QTextFragment).
 * In a document the instances of QTextCharFormat which are based on a
 * KoCharacterStyle have a property StyleId with an integer as value which
 * equals styleId() of that style.
 * @see KoStyleManager
 */
class KOTEXT_EXPORT KoCharacterStyle : public QObject {
    Q_OBJECT
public:
    /// list of character style properties we can store in a QCharFormat
    enum Property {
        StyleId = QTextFormat::UserProperty+1, ///< The id stored in the charFormat to link the text to this style.
        HasHyphenation,
        FontId,
        StrikeOutStyle,
        StrikeOutType,
        StrikeOutColor,
        UnderlineStyle,
        UnderlineType,
        UnderlineColor,
        TransformText,
        Spelling
    };

    /// list of possible line type : no line, single line, double line
    enum LineType {
        NoLineType,
        SingleLine,
        DoubleLine
    };

    /// list of possible line style. 
    enum LineStyle {
        NoLineStyle = Qt::NoPen,
        SolidLine = Qt::SolidLine,
        DottedLine = Qt::DotLine,
        DashLine = Qt::DashLine,
        DotDashLine = Qt::DashDotLine,
        DotDotDashLine = Qt::DashDotDotLine,
        LongDashLine,
        WaveLine
    };

    /// Text transformation
    enum Transform {
        MixedCase = 0, ///< No text-transformation. This is the default value.
        SmallCaps, ///< Small capitalized letters.
        AllUppercase, ///< Uppercase characters. E.g. "My teXT"=>"MY TEXT"
        AllLowercase, ///< Lowercase characters. E.g. "My teXT"=>"my text"
        Capitalize ///< Capitalize characters. E.g. "my text"=>"My Text"
    };

    /**
     * Constructor. Initializes with standard size/font properties.
     * @param parent the parent object for memory management purposes.
     */
    explicit KoCharacterStyle(QObject *parent = 0);
    /// Copy constructor
    KoCharacterStyle(const KoCharacterStyle &other);
    /// Copy constructor
    KoCharacterStyle(const QTextCharFormat &format);
    /// Destructor
    ~KoCharacterStyle();

    /// return the effective font for this style
    QFont font() const;

    /// See similar named method on QTextCharFormat
    void setFontFamily (const QString &family);
    /// See similar named method on QTextCharFormat
    QString fontFamily () const;
    /// See similar named method on QTextCharFormat
    void setFontPointSize (qreal size);
    /// See similar named method on QTextCharFormat
    double fontPointSize () const;
    /// See similar named method on QTextCharFormat
    void setFontWeight (int weight);
    /// See similar named method on QTextCharFormat
    int fontWeight () const;
    /// See similar named method on QTextCharFormat
    void setFontItalic (bool italic);
    /// See similar named method on QTextCharFormat
    bool fontItalic () const;
    /// See similar named method on QTextCharFormat
    void setFontOverline (bool overline);
    /// See similar named method on QTextCharFormat
    bool fontOverline () const;
    /// See similar named method on QTextCharFormat
    void setFontFixedPitch (bool fixedPitch);
    /// See similar named method on QTextCharFormat
    bool fontFixedPitch () const;
    /// See similar named method on QTextCharFormat
    void setVerticalAlignment (QTextCharFormat::VerticalAlignment alignment);
    /// See similar named method on QTextCharFormat
    QTextCharFormat::VerticalAlignment verticalAlignment () const;
    /// See similar named method on QTextCharFormat
    void setTextOutline (const QPen &pen);
    /// See similar named method on QTextCharFormat
    QPen textOutline () const;
    /// See similar named method on QTextCharFormat
    void setFontLetterSpacing(qreal spacing);
    /// See similar named method on QTextCharFormat
    qreal fontLetterSpacing() const;
    /// See similar named method on QTextCharFormat
    void setFontWordSpacing(qreal spacing);
    /// See similar named method on QTextCharFormat
    qreal fontWordSpacing() const;

    /// See similar named method on QTextCharFormat
    void setBackground (const QBrush &brush);
    /// See similar named method on QTextCharFormat
    QBrush background () const;
    /// See similar named method on QTextCharFormat
    void clearBackground ();

    /// See similar named method on QTextCharFormat
    void setForeground (const QBrush &brush);
    /// See similar named method on QTextCharFormat
    QBrush foreground () const;
    /// See similar named method on QTextCharFormat
    void clearForeground ();

    /// Apply a font strike out style to this KoCharacterStyle
    void setStrikeOutStyle (LineStyle style);
    /// Get the current font strike out style of this KoCharacterStyle
    LineStyle strikeOutStyle () const;
    /// Apply a font strike out color to this KoCharacterStyle
    void setStrikeOutColor (const QColor &color);
    /// Get the current font strike out color of this KoCharacterStyle
    QColor strikeOutColor () const;
    /// Apply a font strike out color to this KoCharacterStyle
    void setStrikeOutType (LineType lineType);
    /// Get the current font strike out color of this KoCharacterStyle
    LineType strikeOutType () const;

    /// Apply a font underline style to this KoCharacterStyle
    void setUnderlineStyle (LineStyle style);
    /// Get the current font underline style of this KoCharacterStyle
    LineStyle underlineStyle () const;
    /// Apply a font underline color to this KoCharacterStyle
    void setUnderlineColor (const QColor &color);
    /// Get the current font underline color of this KoCharacterStyle
    QColor underlineColor () const;
    /// Apply a font underline color to this KoCharacterStyle
    void setUnderlineType (LineType lineType);
    /// Get the current font underline color of this KoCharacterStyle
    LineType underlineType () const;


    /// Set the text tranformation.
    void setTransform(Transform transformtext);
    /// Return how the text should be transformed.
    Transform transform() const;

    void setHasHyphenation(bool on);
    bool hasHyphenation() const;

    void copyProperties(const KoCharacterStyle *style);


    /// return the name of the style.
    QString name() const;

    /// set a user-visible name on the style.
    void setName(const QString &name);

    /// each style has a unique ID (non persistent) given out by the styleManager
    int styleId() const;

    /// each style has a unique ID (non persistent) given out by the styleManager
    void setStyleId(int id);

    /**
     * Apply this style to a blockFormat by copying all properties from this
     * style to the target char format.
     */
    void applyStyle(QTextCharFormat &format) const;
    /**
     * Apply this style to the textBlock by copying all properties from this
     * style to the target block formats.
     */
    void applyStyle(QTextBlock &block) const;
    /**
     * Reset any styles and apply this style on the whole selection.
     */
    void applyStyle(QTextCursor *selection) const;

    /**
     * Load the style from the \a KoStyleStack style stack using the
     * OpenDocument format.
     */
    void loadOasis(KoTextLoadingContext& context);

    /// return true if this style has a non-default value set for the Property
    bool hasProperty(int key) const;

    bool operator==( const KoCharacterStyle &other ) const;
    
    void removeDuplicates ( const KoCharacterStyle &other );
    
    void saveOdf ( KoGenStyle *target );
private:
    class Private;
    Private * const d;
};

#endif
