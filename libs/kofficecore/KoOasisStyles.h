/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOOASISSTYLES_H
#define KOOASISSTYLES_H

#include <QHash>
#include <QtXml>

#include <koffice_export.h>
#include <KoXmlReader.h>

class KoGenStyles;
class KoXmlWriter;
class QBrush;
class KoGenStyle;
class KoStyleStack;

/**
 * Repository of styles used during loading of OASIS/OOo file
 */
class KOFFICECORE_EXPORT KoOasisStyles
{
public:
    KoOasisStyles();
    ~KoOasisStyles();

    /// Look into @p doc for styles and remember them
    /// @param doc document to look into
    /// @param stylesDotXml true when loading styles.xml, false otherwise
    void createStyleMap( const KoXmlDocument& doc );

    /**
     * Look up a style by name.
     *  This method can find styles defined by the tags "style:page-layout",
     *   "style:presentation-page-layout", or "style:font-decl".
     * Do NOT use this method for style:style styles.
     *
     * @param name the style name
     * @return the dom element representing the style, or QString::null if it wasn't found.
     */
    const KoXmlElement* findStyle( const QString& name ) const;

    /**
     * Looks up a style:style by name.
     * Searches in the list of custom styles first and then in the list of automatic styles.
     * @param name the style name
     * @param family the style family (for a style:style, use 0 otherwise)
     * @return the dom element representing the style, or QString::null if it wasn't found.
     */
    const KoXmlElement* findStyle( const QString& name, const QString& family ) const;

    /// Similar to findStyle but for custom styles only.
    const KoXmlElement* findStyleCustomStyle( const QString& name, const QString& family ) const;

    /// Similar to findStyle but for auto-styles only.
    const KoXmlElement* findStyleAutoStyle( const QString& name, const QString& family ) const;

    /// @return the default style for a given family ("graphic","paragraph","table" etc.)
    /// Returns 0 if no default style for this family is available
    const KoXmlElement* defaultStyle( const QString& family ) const;

    /// @return the office:style element
    const KoXmlElement& officeStyle() const;

    /// @return all list styles ("text:list-style" elements), hashed by name
    const QHash<QString, KoXmlElement*>& listStyles() const;

    /// @return master pages ("style:master-page" elements), hashed by name
    const QHash<QString, KoXmlElement*>& masterPages() const;

    /// @return draw styles, hashed by name
    const QHash<QString, KoXmlElement*>& drawStyles() const;

    /// @return all custom styles ("style:style" elements) for a given family, hashed by name
    const QHash<QString, KoXmlElement*>& customStyles(const QString& family) const;

    /// @return all auto-styles ("style:style" elements) for a given family, hashed by name
    const QHash<QString, KoXmlElement*>& autoStyles(const QString& family) const;

    /// Prefix and suffix are always included into formatStr. Having them as separate fields simply
    /// allows to extract them from formatStr, to display them in separate widgets.
    struct NumericStyleFormat
    {
        QString formatStr;
        QString prefix;
        QString suffix;
        enum { Number, Scientific, Fraction, Currency, Percentage,
               Date, Time, Boolean, Text } type;
        int precision;
        QString currencySymbol;
    };

    typedef QHash<QString, NumericStyleFormat> DataFormatsMap;
    /// Value (date/time/number...) formats found while parsing styles. Used e.g. for fields.
    /// Key: format name. Value:
    const DataFormatsMap& dataFormats() const;

    static QString saveOasisDateStyle( KoGenStyles &mainStyles, const QString & _format, bool klocaleFormat,
                                       const QString &_prefix = QString::null , const QString &_suffix= QString::null );
    static QString saveOasisTimeStyle( KoGenStyles &mainStyles, const QString & _format, bool klocaleFormat,
                                       const QString &_prefix = QString::null , const QString &_suffix= QString::null );
    static QString saveOasisFractionStyle( KoGenStyles &mainStyles, const QString & _format,
                                           const QString &_prefix = QString::null , const QString &_suffix= QString::null );
    static QString saveOasisScientificStyle( KoGenStyles &mainStyles, const QString & _format,
                                             const QString &_prefix = QString::null , const QString &_suffix= QString::null );
    static QString saveOasisNumberStyle( KoGenStyles &mainStyles, const QString & _format,
                                         const QString &_prefix = QString::null , const QString &_suffix= QString::null );
    static QString saveOasisPercentageStyle( KoGenStyles &mainStyles, const QString & _format,
                                             const QString &_prefix = QString::null , const QString &_suffix= QString::null );
    static QString saveOasisCurrencyStyle( KoGenStyles &mainStyles, const QString & _format, const QString &symbol,
                                           const QString &_prefix = QString::null , const QString &_suffix= QString::null );
    static QString saveOasisTextStyle( KoGenStyles &mainStyles, const QString & _format,
                                       const QString &_prefix = QString::null , const QString &_suffix= QString::null );

    static void saveOasisFillStyle( KoGenStyle &styleFill, KoGenStyles& mainStyles, const QBrush & brush );
    static QString saveOasisHatchStyle( KoGenStyles& mainStyles, const QBrush &brush );

    static QBrush loadOasisFillStyle( const KoStyleStack &styleStack, const QString & fill,  const KoOasisStyles & oasisStyles );

private:
    /// Add styles to styles map
    void insertStyles( const KoXmlElement& styles, bool styleAutoStyles = false );

private:
    void insertOfficeStyles( const KoXmlElement& styles );
    void insertStyle( const KoXmlElement& style, bool styleAutoStyles );
    void importDataStyle( const KoXmlElement& parent );
    static bool saveOasisTimeFormat( KoXmlWriter &elementWriter, QString & format, QString & text, bool &antislash );
    static void parseOasisDateKlocale(KoXmlWriter &elementWriter, QString & format, QString & text );
    static bool saveOasisKlocaleTimeFormat( KoXmlWriter &elementWriter, QString & format, QString & text );
    static void parseOasisTimeKlocale(KoXmlWriter &elementWriter, QString & format, QString & text );
    static void addKofficeNumericStyleExtension( KoXmlWriter & elementWriter, const QString &_suffix, const QString &_prefix );

    KoOasisStyles( const KoOasisStyles & ); // forbidden
    KoOasisStyles& operator=( const KoOasisStyles & ); // forbidden

private:
    class Private;
    Private * const d;
};

#endif /* KOOASISSTYLES_H */
