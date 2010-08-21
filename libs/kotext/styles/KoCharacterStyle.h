/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2008 Pierre Stirnweiss <pierre.stirnweiss_koffice@gadz.org>
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
#include "kotext_export.h"


class StylePrivate;
class QTextBlock;
class KoStyleStack;
class KoOdfLoadingContext;
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
class KOTEXT_EXPORT KoCharacterStyle : public QObject
{
    Q_OBJECT
public:
    /// list of character style properties we can store in a QTextCharFormat
    enum Property {
        StyleId = QTextFormat::UserProperty + 1, ///< The id stored in the charFormat to link the text to this style.
        HasHyphenation,
        StrikeOutStyle,
        StrikeOutType,
        StrikeOutColor,
        StrikeOutWidth,
        StrikeOutWeight,
        StrikeOutMode,
        StrikeOutText,
        UnderlineStyle,
        UnderlineType,
        UnderlineWidth,
        UnderlineWeight,
        UnderlineMode,
        Language,
        Country,
        FontCharset,
        TextRotationAngle,
        TextRotationScale,
        TextScale,
        InlineRdf,  ///< KoTextInlineRdf pointer
        InlineInstanceId = 577297549, // Internal: Reserved for KoInlineTextObjectManager
        ChangeTrackerId = 577297550 // Internal: Reserved for ChangeTracker
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

    enum LineWeight {
        AutoLineWeight,
        NormalLineWeight,
        BoldLineWeight,
        ThinLineWeight,
        DashLineWeight, // ## ??what the heck does this mean??
        MediumLineWeight,
        ThickLineWeight,
        PercentLineWeight,
        LengthLineWeight
    };

    /// list of possible line modes.
    enum LineMode {
        NoLineMode,
        ContinuousLineMode,
        SkipWhiteSpaceLineMode
    };

    enum RotationAngle {
        Zero,
        Ninety = 90,
        TwoHundredSeventy = 270
    };

    enum RotationScale {
        Fixed,
        LineHeight
    };

    /**
     * Constructor. Initializes with standard size/font properties.
     * @param parent the parent object for memory management purposes.
     */
    explicit KoCharacterStyle(QObject *parent = 0);
    /// Copy constructor
    explicit KoCharacterStyle(const QTextCharFormat &format, QObject *parent = 0);
    /// Destructor
    ~KoCharacterStyle();

    /// return the effective font for this style
    QFont font() const;

    /// See similar named method on QTextCharFormat
    void setFontFamily(const QString &family);
    /// See similar named method on QTextCharFormat
    QString fontFamily() const;
    /// See similar named method on QTextCharFormat
    void setFontPointSize(qreal size);
    /// See similar named method on QTextCharFormat
    qreal fontPointSize() const;
    /// See similar named method on QTextCharFormat
    void setFontWeight(int weight);
    /// See similar named method on QTextCharFormat
    int fontWeight() const;
    /// See similar named method on QTextCharFormat
    void setFontItalic(bool italic);
    /// See similar named method on QTextCharFormat
    bool fontItalic() const;
    /// See similar named method on QTextCharFormat
    void setFontOverline(bool overline);
    /// See similar named method on QTextCharFormat
    bool fontOverline() const;
    /// See similar named method on QTextCharFormat
    void setFontFixedPitch(bool fixedPitch);
    /// See similar named method on QTextCharFormat
    bool fontFixedPitch() const;
    /// See similar named method on QTextCharFormat
    void setVerticalAlignment(QTextCharFormat::VerticalAlignment alignment);
    /// See similar named method on QTextCharFormat
    QTextCharFormat::VerticalAlignment verticalAlignment() const;
    /// See similar named method on QTextCharFormat
    void setTextOutline(const QPen &pen);
    /// See similar named method on QTextCharFormat
    QPen textOutline() const;
    /// See similar named method on QTextCharFormat
    void setFontLetterSpacing(qreal spacing);
    /// See similar named method on QTextCharFormat
    qreal fontLetterSpacing() const;
    /// See similar named method on QTextCharFormat
    void setFontWordSpacing(qreal spacing);
    /// See similar named method on QTextCharFormat
    qreal fontWordSpacing() const;
    /// Set the text capitalization
    void setFontCapitalization(QFont::Capitalization capitalization);
    /// Return how the text should be capitalized
    QFont::Capitalization fontCapitalization() const;

    /// See similar named method on QTextCharFormat
    void setFontStyleHint(QFont::StyleHint styleHint);
    /// See similar named method on QTextCharFormat
    QFont::StyleHint fontStyleHint() const;
    /// See similar named method on QTextCharFormat
    void setFontKerning(bool enable);
    /// See similar named method on QTextCharFormat
    bool fontKerning() const;

    /// See similar named method on QTextCharFormat
    void setBackground(const QBrush &brush);
    /// See similar named method on QTextCharFormat
    QBrush background() const;
    /// See similar named method on QTextCharFormat
    void clearBackground();

    /// See similar named method on QTextCharFormat
    void setForeground(const QBrush &brush);
    /// See similar named method on QTextCharFormat
    QBrush foreground() const;
    /// See similar named method on QTextCharFormat
    void clearForeground();

    /// Apply a font strike out style to this KoCharacterStyle
    void setStrikeOutStyle(LineStyle style);
    /// Get the current font strike out style of this KoCharacterStyle
    LineStyle strikeOutStyle() const;
    /// Apply a font strike out width to this KoCharacterStyle
    void setStrikeOutWidth(LineWeight weight, qreal width);
    /// Get the current font strike out width of this KoCharacterStyle
    void strikeOutWidth(LineWeight &weight, qreal &width) const;
    /// Apply a font strike out color to this KoCharacterStyle
    void setStrikeOutColor(const QColor &color);
    /// Get the current font strike out color of this KoCharacterStyle
    QColor strikeOutColor() const;
    /// Apply a font strike out color to this KoCharacterStyle
    void setStrikeOutType(LineType lineType);
    /// Get the current font strike out color of this KoCharacterStyle
    LineType strikeOutType() const;
    /// Apply a strike out mode of this KoCharacterStyle
    void setStrikeOutMode(LineMode lineMode);
    /// Get the current strike out mode of this KoCharacterStyle
    LineMode strikeOutMode() const;
    /// Apply a strike out text of this KoCharacterStyle
    void setStrikeOutText(const QString &text);
    /// Get the current strike out text of this KoCharacterStyle
    QString strikeOutText() const;

    /// Apply a font underline style to this KoCharacterStyle
    void setUnderlineStyle(LineStyle style);
    /// Get the current font underline style of this KoCharacterStyle
    LineStyle underlineStyle() const;
    /// Apply a font underline width to this KoCharacterStyle
    void setUnderlineWidth(LineWeight weight, qreal width);
    /// Get the current font underline width of this KoCharacterStyle
    void underlineWidth(LineWeight &weight, qreal &width) const;
    /// Apply a font underline color to this KoCharacterStyle
    void setUnderlineColor(const QColor &color);
    /// Get the current font underline color of this KoCharacterStyle
    QColor underlineColor() const;
    /// Apply a font underline color to this KoCharacterStyle
    void setUnderlineType(LineType lineType);
    /// Get the current font underline color of this KoCharacterStyle
    LineType underlineType() const;
    /// Apply a underline mode to this KoCharacterStyle
    void setUnderlineMode(LineMode mode);
    /// Get the current underline mode of this KoCharacterStyle
    LineMode underlineMode() const;

    /// Apply text rotation angle to this KoCharacterStyle
    void setTextRotationAngle(RotationAngle angle);
    /// Get the current text rotation angle of this KoCharacterStyle
    RotationAngle textRotationAngle() const;
    /**
     *  RotationScale pecifies whether for rotated text the width of the text
     *  should be scaled to fit into the current line height or the width of the text
     *  should remain fixed, therefore changing the current line height
     */
    void setTextRotationScale(RotationScale scale);
    /// Get the current text rotation scale of this KoCharacterStyle
    RotationScale textRotationScale() const;
    /// Apply text scale to this KoCharacterStyle
    void setTextScale(int scale);
    /// Get the current text scale of this KoCharacterStyle
    int textScale() const;

    /// Set the country
    void setCountry(const QString &country);
    /// Set the language
    void setLanguage(const QString &language);
    /// Get the country
    QString country() const;
    /// Get the language
    QString language() const;

    void setHasHyphenation(bool on);
    bool hasHyphenation() const;

    void copyProperties(const KoCharacterStyle *style);
    void copyProperties(const QTextCharFormat &format);

    KoCharacterStyle *clone(QObject *parent = 0);

    /// return the name of the style.
    QString name() const;

    /// set a user-visible name on the style.
    void setName(const QString &name);

    /// each style has a unique ID (non persistent) given out by the styleManager
    int styleId() const;

    /// each style has a unique ID (non persistent) given out by the styleManager
    void setStyleId(int id);

    void unapplyStyle(QTextBlock &block) const;
    void unapplyStyle(QTextCharFormat &format) const;

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
    void loadOdf(KoOdfLoadingContext &context);

    /// return true if this style has a non-default value set for the Property
    bool hasProperty(int key) const;

    bool operator==(const KoCharacterStyle &other) const;

    /**
     * Removes properties from this style that have the same value in other style.
     */
    void removeDuplicates(const KoCharacterStyle &other);

    /**
     * Removes properties from this style that have the same value in other format.
     */
    void removeDuplicates(const QTextCharFormat &other_format);

    void saveOdf(KoGenStyle &style);

    /**
     * Returns true if this style has no properties set. Else, returns false.
     */
    bool isEmpty() const;

    /**
     * Return the value of key as represented on this style.
     * You should consider using the direct accessors for individual properties instead.
     * @param key the Property to request.
     * @returns a QVariant which holds the property value.
     */
    QVariant value(int key) const;

    /**
     * Remove the hardcoded defaults from this style (SansSerif, 12 points, black).
     * @internal - this method is a bit of an ugly workaround to make it easier to
     * use KoTextLoader for loading richtext in kspread, normally styles with
     * no font etc. set are not something you should want.
     */
    void removeHardCodedDefaults();
signals:
    void nameChanged(const QString &newName);

private:
    /**
    * Load the style from the \a KoStyleStack style stack using the
    * OpenDocument format.
    */
    void loadOdfProperties(KoStyleStack &styleStack);

    class Private;
    Private * const d;
};

#endif
