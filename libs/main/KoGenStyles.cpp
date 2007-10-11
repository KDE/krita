/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoGenStyles.h"
#include <KoXmlWriter.h>
#include <float.h>
#include <kdebug.h>

static const struct {
    KoGenStyle::Type m_type;
    const char * m_elementName;
    const char * m_propertiesElementName;
    bool m_drawElement;
} styleData[] = {
    { KoGenStyle::StyleUser, "style:style", "style:paragraph-properties", false  },
    { KoGenStyle::StyleTableColumn, "style:style", "style:table-column-properties", false  },
    { KoGenStyle::StyleTableRow, "style:style", "style:table-row-properties", false  },
    { KoGenStyle::StyleTableCell, "style:style", "style:table-cell-properties", false  },
    { KoGenStyle::StyleList, "text:list-style", 0, false  },
    { KoGenStyle::StyleGradientLinear, "svg:linearGradient", 0, true  },
    { KoGenStyle::StyleGradientRadial, "svg:radialGradient", 0, true  },
    { KoGenStyle::StyleStrokeDash, "draw:stroke-dash", 0, true  },
    { KoGenStyle::StyleFillImage, "draw:fill-image", 0, true  },
    { KoGenStyle::StyleHatch, "draw:hatch", "style:graphic-properties", true  },
    { KoGenStyle::StyleGradient, "draw:gradient", "style:graphic-properties", true  },
    { KoGenStyle::StyleMarker, "draw:marker", "style:graphic-properties", true  }
};

static const unsigned int numStyleData = sizeof( styleData ) / sizeof( *styleData );

static const struct {
    KoGenStyle::Type m_type;
    const char * m_elementName;
    const char * m_propertiesElementName;
    bool m_drawElement;
} autoStyleData[] = {
    { KoGenStyle::StyleAuto, "style:style", "style:paragraph-properties", false  },
    { KoGenStyle::StyleGraphicAuto, "style:style", "style:graphic-properties", false  },
    { KoGenStyle::StyleDrawingPage, "style:style", "style:drawing-page-properties", false  },
    { KoGenStyle::StyleAutoTable, "style:style", "style:table-properties", false  },
    { KoGenStyle::StyleAutoTableColumn, "style:style", "style:table-column-properties", false  },
    { KoGenStyle::StyleAutoTableRow, "style:style", "style:table-row-properties", false  },
    { KoGenStyle::StyleAutoTableCell, "style:style", "style:table-cell-properties", false  },
    { KoGenStyle::StylePageLayout, "style:page-layout", "style:page-layout-properties", false  },
    { KoGenStyle::StyleAutoList, "text:list-style", 0, false  },
    { KoGenStyle::StyleNumericNumber, "number:number-style", 0, false  },
    { KoGenStyle::StyleNumericFraction, "number:number-style", 0, false  },
    { KoGenStyle::StyleNumericScientific, "number:number-style", 0, false  },
    { KoGenStyle::StyleNumericDate, "number:date-style", 0, false  },
    { KoGenStyle::StyleNumericTime, "number:time-style", 0, false  },
    { KoGenStyle::StyleNumericPercentage, "number:percentage-style", 0, false  },
    { KoGenStyle::StyleNumericCurrency, "number:currency-style", 0, false  },
    { KoGenStyle::StyleNumericBoolean, "number:boolean-style", 0, false  },
    { KoGenStyle::StyleNumericText, "number:text-style", 0, false  }
};

static const unsigned int numAutoStyleData = sizeof( autoStyleData ) / sizeof( *autoStyleData );

class KoGenStyles::Private
{
};

KoGenStyles::KoGenStyles()
    : d( 0 )
{
}

KoGenStyles::~KoGenStyles()
{
    delete d;
}

QString KoGenStyles::lookup( const KoGenStyle& style, const QString& name, int flags )
{
    StyleMap::iterator it = m_styleMap.find( style );
    if ( it == m_styleMap.end() ) {
        // Not found, try if this style is in fact equal to its parent (the find above
        // wouldn't have found it, due to m_parentName being set).
        if ( !style.parentName().isEmpty() ) {
            KoGenStyle testStyle( style );
            const KoGenStyle* parentStyle = this->style( style.parentName() ); // ## linear search
            if( !parentStyle ) {
                kDebug(30003) <<"KoGenStyles::lookup(" << name <<"): parent style '" << style.parentName() <<"' not found in collection";
            } else {
                if ( testStyle.m_familyName != parentStyle->m_familyName )
                {
                    kWarning(30003) << "KoGenStyles::lookup(" << name << ", family=" << testStyle.m_familyName << ") parent style '" << style.parentName() << "' has a different family: " << parentStyle->m_familyName;
                }

                testStyle.m_parentName = parentStyle->m_parentName;
                // Exclude the type from the comparison. It's ok for an auto style
                // to have a user style as parent; they can still be identical
                testStyle.m_type = parentStyle->m_type;
                // Also it's ok to not have the display name of the parent style
                // in the auto style
                QMap<QString, QString>::const_iterator it = parentStyle->m_attributes.find( "style:display-name" );
                if ( it != parentStyle->m_attributes.end() )
                    testStyle.addAttribute( "style:display-name", *it );

                if ( *parentStyle == testStyle )
                    return style.parentName();
            }
        }

        QString styleName( name );
        if ( styleName.isEmpty() ) {
            styleName = 'A'; // for "auto".
            flags &= ~DontForceNumbering; // i.e. force numbering
        }
        styleName = makeUniqueName( styleName, flags );
        if ( style.autoStyleInStylesDotXml() )
            m_autoStylesInStylesDotXml.insert( styleName );
        else
            m_styleNames.insert( styleName );
        it = m_styleMap.insert( style, styleName );
        NamedStyle s;
        s.style = &it.key();
        s.name = styleName;
        m_styleArray.append( s );
    }
    return it.value();
}

QString KoGenStyles::makeUniqueName( const QString& base, int flags ) const
{
    // If this name is not used yet, and numbering isn't forced, then the given name is ok.
    if ( ( flags & DontForceNumbering )
         && ! m_autoStylesInStylesDotXml.contains( base )
         && ! m_styleNames.contains( base ) )
        return base;
    int num = 1;
    QString name;
    do {
        name = base;
        name += QString::number( num++ );
    } while ( m_autoStylesInStylesDotXml.contains( name )
              || m_styleNames.contains( name ) );
    return name;
}

QList<KoGenStyles::NamedStyle> KoGenStyles::styles( int type, bool markedForStylesXml ) const
{
    QList<KoGenStyles::NamedStyle> lst;
    const NameMap& nameMap = markedForStylesXml ? m_autoStylesInStylesDotXml : m_styleNames;
    StyleArray::const_iterator it = m_styleArray.begin();
    const StyleArray::const_iterator end = m_styleArray.end();
    for ( ; it != end ; ++it ) {
        // Look up if it's marked for styles.xml or not by looking up in the corresponding style map.
        if ( (*it).style->type() == type && nameMap.find((*it).name) != nameMap.end() ) {
            lst.append( *it );
        }
    }
    return lst;
}

const KoGenStyle* KoGenStyles::style( const QString& name ) const
{
    StyleArray::const_iterator it = m_styleArray.begin();
    const StyleArray::const_iterator end = m_styleArray.end();
    for ( ; it != end ; ++it ) {
        if ( (*it).name == name )
            return (*it).style;
    }
    return 0;
}

KoGenStyle* KoGenStyles::styleForModification( const QString& name )
{
    return const_cast<KoGenStyle *>( style( name ) );
}

void KoGenStyles::markStyleForStylesXml( const QString& name )
{
    Q_ASSERT( m_styleNames.contains( name ) );
    m_styleNames.remove( name );
    m_autoStylesInStylesDotXml.insert( name );
    styleForModification( name )->setAutoStyleInStylesDotXml( true );
}

void KoGenStyles::dump()
{
    kDebug(30003) <<"Style array:";
    StyleArray::const_iterator it = m_styleArray.begin();
    const StyleArray::const_iterator end = m_styleArray.end();
    for ( ; it != end ; ++it ) {
        kDebug(30003) << (*it).name;
    }
    for ( NameMap::const_iterator it = m_styleNames.begin(); it != m_styleNames.end(); ++it ) {
        kDebug(30003) <<"style:" << *it;
    }
    for ( NameMap::const_iterator it = m_autoStylesInStylesDotXml.begin(); it != m_autoStylesInStylesDotXml.end(); ++it ) {
        kDebug(30003) <<"auto style for style.xml:" << *it;
        const KoGenStyle* s = style( *it );
        Q_ASSERT( s );
        Q_ASSERT( s->autoStyleInStylesDotXml() );
    }
}

void KoGenStyles::saveOdfAutomaticStyles( KoXmlWriter* xmlWriter, bool stylesDotXml )
{
    xmlWriter->startElement( "office:automatic-styles" );

    for ( int i = 0; i < numAutoStyleData; ++i ) {
        QList<KoGenStyles::NamedStyle> stylesList = styles( int( autoStyleData[i].m_type ), stylesDotXml );
        QList<KoGenStyles::NamedStyle>::const_iterator it = stylesList.begin();
        for ( ; it != stylesList.end() ; ++it ) {
            ( *it ).style->writeStyle( xmlWriter, *this, autoStyleData[i].m_elementName, ( *it ).name,
                                       autoStyleData[i].m_propertiesElementName, true, autoStyleData[i].m_drawElement );
        }
    }

    xmlWriter->endElement(); // office:automatic-styles
}


void KoGenStyles::saveOdfDocumentStyles( KoXmlWriter* xmlWriter )
{
    xmlWriter->startElement( "office:styles" );

    for ( int i = 0; i < numStyleData; ++i ) {
        QList<KoGenStyles::NamedStyle> stylesList = styles( int( styleData[i].m_type ) );
        QList<KoGenStyles::NamedStyle>::const_iterator it = stylesList.begin();
        for ( ; it != stylesList.end() ; ++it ) {
            ( *it ).style->writeStyle( xmlWriter, *this, styleData[i].m_elementName, ( *it ).name,
                                       styleData[i].m_propertiesElementName, true, styleData[i].m_drawElement );
        }
    }

    xmlWriter->endElement(); // office:styles
}

void KoGenStyles::saveOdfMasterStyles( KoXmlWriter* xmlWriter )
{
    xmlWriter->startElement( "office:master-styles" );

    QList<KoGenStyles::NamedStyle> stylesList = styles( KoGenStyle::StyleMaster );
    QList<KoGenStyles::NamedStyle>::const_iterator it = stylesList.begin();
    for ( ; it != stylesList.end() ; ++it ) {
        ( *it ).style->writeStyle( xmlWriter, *this, "style:master-page", ( *it ).name, 0 );
    }

    xmlWriter->endElement(); // office:styles
}

