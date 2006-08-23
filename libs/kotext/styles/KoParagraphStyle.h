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
class KoListStyle;
class StylePrivate;
class QTextBlock;

/**
 * A container for all properties for the paragraph wide style.
 * Each paragraph in the main text either is based on a parag style, or its not. Where
 * it is based on a paragraph style this is indecated that it has a property 'StyleId'
 * with an integer as value.  The integer value corresponds to the styleId() output of
 * a specific KoParagraphStyle.
 * @see KoStyleManager
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
        LineSpacingFromFont,  ///< if false, use fontsize (in pt) solely, otherwise respect font settings
        AlignLastLine,      ///< When the paragraph is justified, what to do with the last word line
        WidowThreshold,     ///< If 'keep together'=false, amount of lines to keep it anyway.
        OrphanThreshold,   ///< If 'keep together'=false, amount of lines to keep it anyway.
        DropCaps,       ///< defines if a paragraph renders its first char(s) with drop-caps
        DropCapsLength, ///< Number of glyphs to show as drop-caps
        DropCapsLines,  ///< Number of lines that the drop-caps span
        DropCapsDistance,   ///< Distance between drop caps and text
        FollowDocBaseline,  ///< If true the baselines will be aligned with the doc-wide grid
        BreakBefore,    ///< If true, insert a frame break before this paragraph
        BreakAfter,     ///< If true, insert a frame break after this paragraph
        HasLeftBorder,  ///< If true, paint a border on the left
        HasTopBorder,   ///< If true, paint a border on the top
        HasRightBorder, ///< If true, paint a border on the right
        HasBottomBorder,///< If true, paint a border on the bottom
        BorderLineWidth,///< Thickness of inner-border
        SecondBorderLineWidth,  ///< Thickness of outer-border
        DistanceToSecondBorder, ///< Distance between inner and outer border
        LeftPadding,    ///< distance between text and border
        TopPadding,     ///< distance between text and border
        RightPadding,   ///< distance between text and border
        BottomPadding   ///< distance between text and border
// continue at 15.5.28
    };

    KoParagraphStyle();
    /// Copy constructor
    KoParagraphStyle(const KoParagraphStyle &orig);
    ~KoParagraphStyle();

    //  ***** Linespacing
    /**
     * Sets the line height as a percentage of the highest character on that line.
     * A good typographically correct value would be 120%
     * @see setFontIndependentLineSpacing
     */
    void setLineHeightPercent(int lineHeight)
        {setProperty(FixedLineHeight, lineHeight); remove(LineSpacing);}
    int lineHeightPercent() const { return propertyInt(FixedLineHeight); }

    /**
     * Sets the line height to a specific pt-based height, ignoring the font size.
     */
    void setLineHeightAbsolute(double height)
        {setProperty(FixedLineHeight, height); remove(LineSpacing);}
    double lineHeightAbsolute() const {return propertyDouble(FixedLineHeight); }

    /**
     * Sets the line height to have a minimum height in pt.
     */
    void setMinimumLineHeight(double height) {setProperty(MinimumLineHeight, height); }
    double minimumLineHeight() const { return propertyDouble(MinimumLineHeight); }

    /**
     * Sets the space between two lines to be a specific height, ignoring the font size.
     */
    void setLineSpacing(double spacing)
        {setProperty(LineSpacing, spacing); remove(FixedLineHeight); }
    double lineSpacing() const {return propertyDouble(LineSpacing); }

    /**
     * If set to true the font-encoded height will be used instead of the font-size propery
     * This property influences setLineHeightPercent() behavior.
     * When off (default) a font of 12pt will always have a linespacing of 12pt times the
     * current linespacing percentage.  When on the linespacing embedded in the font
     * is used which can differ for various fonts, even if they are the same pt-size.
     */
    void setLineSpacingFromFont(bool on) {setProperty(LineSpacingFromFont, on); }
    /**
     * @see setFontIndependentLineSpacing
     */
    bool lineSpacingFromFont() const {return propertyBoolean(LineSpacingFromFont); }


    /**
     * For paragraphs that are justified the last line alignment is specified here.
     * There are only 3 valid options, Left, Center and Justified. (where Left will
     * be right aligned for RTL text).
     */
    void setAlignLastLine(Qt::Alignment alignment ) { setProperty(AlignLastLine, (int) alignment); }
    /**
     * @see setAlignLastLine
     */
    Qt::Alignment alignLastLine() const {
        return static_cast<Qt::Alignment> (propertyInt(QTextFormat::BlockAlignment));
    }
    /**
     * Paragraphs that are broken accross two frames are normally broken at the bottom
     * of the frame.  Using this property we can set the minimum number of lines that should
     * appear in the second frame to avoid really short paragraphs standing alone (also called
     * widows).  So, if a 10 line parag is broken in a way that only one line is in the second
     * frame, setting a widowThreshold of 4 will break at 6 lines instead to leave the
     * requested 4 lines.
     */
    void setWidowThreshold(int lines) { setProperty(WidowThreshold, lines); }
    /**
     * @see setWidowThreshold
     */
    int widowThreshold() const { return propertyInt(WidowThreshold); }
    /**
     * Paragraphs that are broken accross two frames are normally broken at the bottom
     * of the frame.  Using this property we can set the minimum number of lines that should
     * appear in the first frame to avoid really short paragraphs standing alone (also called
     * orphans).  So, if a paragraph is broken so only 2 line is left in the first frame
     * setting the orphanThreshold to something greater than 2 will move the whole paragraph
     * to the second frame.
     */
    void setOrphanThreshold(int lines) { setProperty(OrphanThreshold, lines); }
    /**
     * @see setOrphanThreshold
     */
    int orphanThreshold() const { return propertyInt(OrphanThreshold); }
    /**
     * If true, make the first character span multiple lines.
     * @see setDropCapsLenght
     * @see setDropCapsLines
     * @see dropCapsDistance
     */
    void setDropCaps(bool on) { setProperty(DropCaps, on); }
    /**
     * @see setDropCaps
     */
    bool dropCaps() const { return propertyBoolean(DropCaps); }
    /**
     * Set the number of glyphs to show as drop-caps
     * @see setDropCaps
     * @see setDropCapsLines
     * @see dropCapsDistance
     */
    void setDropCapsLenght(int characters) { setProperty(DropCapsLength, characters); }
    /**
     * set dropCaps Lenght in characters
     * @see setDropCapsLenght
     */
    int dropCapsLength() const { return propertyInt(DropCapsLength); }
    /**
     * Set the number of lines that the drop-caps span
     * @see setDropCapsLenght
     * @see setDropCaps
     * @see dropCapsDistance
     */
    void setDropCapsLines(int lines) { setProperty(DropCapsLines, lines); }
    /**
     * set dropCapsLines
     * @see setDropCapsLines
     */
    int dropCapsLines() const { return propertyInt(DropCapsLines); }
    /**
     * set the distance between drop caps and text in pt
     * @see setDropCapsLenght
     * @see setDropCaps
     * @see setDropCapsLines
     */
    void setDropCapsDistance(double distance) { setProperty(DropCapsDistance, distance); }
    /**
     * Set dropCaps distance
     * @see setDropCapsDistance
     */
    double dropCapsDistance() const { return propertyDouble(DropCapsDistance); }
    /**
     * If true the baselines will be aligned with the doc-wide grid
     */
    void setFollowDocBaseline(bool on) { setProperty(FollowDocBaseline, on); }
    /**
     * return if baseline alignment is used
     * @see setFollowDocBaseline
     */
    bool followDocBaseline() const { return propertyBoolean(FollowDocBaseline); }

    void setBreakBefore(bool on) { setProperty(BreakBefore, on); }
    bool breakBefore() { return propertyBoolean(BreakBefore); }
    void setBreakAfter(bool on) { setProperty(BreakAfter, on); }
    bool breakAfter() { return propertyBoolean(BreakAfter); }
    void setHasLeftBorder(bool on) { setProperty(HasLeftBorder, on); }
    bool hasLeftBorder() { return propertyBoolean(HasLeftBorder); }
    void setHasTopBorder(bool on) { setProperty(HasTopBorder, on); }
    bool hasTopBorder() { return propertyBoolean(HasTopBorder); }
    void setHasRightBorder(bool on) { setProperty(HasRightBorder, on); }
    bool hasRightBorder() { return propertyBoolean(HasRightBorder); }
    void setHasBottomBorder(bool on) { setProperty(HasBottomBorder, on); }
    bool hasBottomBorder() { return propertyBoolean(HasBottomBorder); }
    void setBorderLineWidth(double width) { setProperty(BorderLineWidth, width); }
    double borderLineWidth() { return propertyDouble(BorderLineWidth); }
    void setSecondBorderLineWidth(double width) { setProperty(SecondBorderLineWidth, width); }
    double secondBorderLineWidth() { return propertyDouble(SecondBorderLineWidth); }
    void setDistanceToSecondBorder(double distance) { setProperty(DistanceToSecondBorder, distance); }
    double distanceToSecondBorder() { return propertyDouble(DistanceToSecondBorder); }
    void setLeftPadding(double padding) { setProperty(LeftPadding, padding); }
    double leftPadding() { return propertyDouble(LeftPadding); }
    void setTopPadding(double padding) { setProperty(TopPadding, padding); }
    double topPadding() { return propertyDouble(TopPadding); }
    void setRightPadding(double padding) { setProperty(RightPadding, padding); }
    double rightPadding() { return propertyDouble(RightPadding); }
    void setBottomPadding(double padding) { setProperty(BottomPadding, padding); }
    double bottomPadding() { return propertyDouble(BottomPadding); }




    // ************ properties from QTextFormat
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
    /// duplicated property from QTextBlockFormat
    void setNonBreakableLines(bool on) { setProperty(QTextFormat::BlockNonBreakableLines, on); }
    /// duplicated property from QTextBlockFormat
    bool nonBreakableLines() const { return propertyBoolean(QTextFormat::BlockNonBreakableLines); }

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

    KoCharacterStyle *characterStyle() { return m_charStyle; }
    const KoCharacterStyle *characterStyle() const { return m_charStyle; }

    KoListStyle *listStyle() { return m_listStyle; }
    const KoListStyle *listStyle() const { return m_listStyle; }

    void setListStyle(const KoListStyle &style);

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
    KoListStyle *m_listStyle;
    KoParagraphStyle *m_parent;
    int m_next;
    StylePrivate *m_stylesPrivate;
};

#endif
