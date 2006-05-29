/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef XMLWRITER_H
#define XMLWRITER_H

#include <QString>
#include <QStack>
#include <QMap>
#include <QIODevice>
#include <koffice_export.h>


/**
 * A class for writing out XML (to any QIODevice), with a special attention on performance.
 * The XML is being written out along the way, which avoids requiring the entire
 * document in memory (like QDom does), and avoids using QTextStream at all
 * (which in Qt3 has major performance issues when converting to utf8).
 */
class KOSTORE_EXPORT KoXmlWriter
{
public:
    /**
     * Create a KoXmlWriter instance to write out an XML document into
     * the given QIODevice.
     */
    KoXmlWriter( QIODevice* dev, int indentLevel = 0 );

    /// Destructor
    ~KoXmlWriter();

    QIODevice *device() const { return m_dev; }

    /**
     * Start the XML document.
     * This writes out the \<?xml?\> tag with utf8 encoding, and the DOCTYPE.
     * @param rootElemName the name of the root element, used in the DOCTYPE tag.
     * @param publicId the public identifier, e.g. "-//OpenOffice.org//DTD OfficeDocument 1.0//EN"
     * @param systemId the system identifier, e.g. "office.dtd" or a full URL to it.
     */
    void startDocument( const char* rootElemName, const char* publicId = 0, const char* systemId = 0 );

    /// Call this to terminate an XML document.
    void endDocument();

    /**
     * Start a new element, as a child of the current element.
     * @param tagName the name of the tag. Warning: this string must
     * remain alive until endElement, no copy is internally made.
     * Usually tagName is a string constant so this is no problem anyway.
     * @param indentInside if set to false, there will be no indentation inside
     * this tag. This is useful for elements where whitespace matters.
     */
    void startElement( const char* tagName, bool indentInside = true );

    /**
     * Overloaded version of addAttribute( const char*, const char* ),
     * which is a bit slower because it needs to convert @p value to utf8 first.
     */
    inline void addAttribute( const char* attrName, const QString& value ) {
        addAttribute( attrName, value.toUtf8() );
    }
    /**
     * Add an attribute whose value is an integer
     */
    inline void addAttribute( const char* attrName, int value ) {
        addAttribute( attrName, QByteArray::number( value ) );
    }
    /**
     * Add an attribute whose value is an unsigned integer
     */
    inline void addAttribute( const char* attrName, uint value ) {
        addAttribute( attrName, QByteArray::number( value ) );
    }
    /**
     * Add an attribute whose value is a floating point number
     * The number is written out with the highest possible precision
     * (unlike QString::number and setNum, which default to 6 digits)
     */
    void addAttribute( const char* attrName, double value );
    /**
     * Add an attribute which represents a distance, measured in pt
     * The number is written out with the highest possible precision
     * (unlike QString::number and setNum, which default to 6 digits),
     * and the unit name ("pt") is appended to it.
     */
    void addAttributePt( const char* attrName, double value );

    /// Overloaded version of the one taking a const char* argument, for convenience
    void addAttribute( const char* attrName, const QByteArray& value );

    /**
     * Add an attribute to the current element.
     */
    void addAttribute( const char* attrName, const char* value );
    /**
     * Terminate the current element. After this you should start a new one (sibling),
     * add a sibling text node, or close another one (end of siblings).
     */
    void endElement();
    /**
     * Overloaded version of addTextNode( const char* ),
     * which is a bit slower because it needs to convert @p str to utf8 first.
     */
    inline void addTextNode( const QString& str ) {
        addTextNode( str.toUtf8() );
    }
    /// Overloaded version of the one taking a const char* argument
    void addTextNode( const QByteArray& cstr );
    /**
     * @brief Adds a text node as a child of the current element.
     *
     * This is appends the litteral content of @p str to the contents of the element.
     * E.g. addTextNode( "foo" ) inside a \<p\> element gives \<p\>foo\</p\>,
     * and startElement( "b" ); endElement( "b" ); addTextNode( "foo" ) gives \<p\>\<b/\>foo\</p\>
     */
    void addTextNode( const char* cstr );

    /**
     * @brief Adds a processing instruction
     *
     * This writes a processing instruction, like <?foo bar blah?>, where foo
     * is the target, and the rest is the data.
     *
     * Processing instructions are used in XML to keep processor-specific
     * information in the text of the document.
     */
    void addProcessingInstruction( const char* cstr );

    /**
     * This is quite a special-purpose method, not for everyday use.
     * It adds a complete element (with its attributes and child elements)
     * as a child of the current element. The string is supposed to be escaped
     * for XML already, so it will usually come from another KoXmlWriter.
     */
    void addCompleteElement( const char* cstr );

    /**
     * This is quite a special-purpose method, not for everyday use.
     * It adds a complete element (with its attributes and child elements)
     * as a child of the current element. The iodevice is supposed to be escaped
     * for XML already, so it will usually come from another KoXmlWriter.
     * This is usually used with KTempFile.
     */
    void addCompleteElement( QIODevice* dev );

    // #### Maybe we want to subclass KoXmlWriter for manifest files.
    /**
     * Special helper for writing "manifest" files
     * This is equivalent to startElement/2*addAttribute/endElement
     * This API will probably have to change (or not be used anymore)
     * when we add support for encrypting/signing.
     * @note OASIS-specific
     */
    void addManifestEntry( const QString& fullPath, const QString& mediaType );

    /**
     * Special helper for writing config item into settings.xml
     * @note OASIS-specific
     */
    void addConfigItem( const QString & configName, const QString& value );
    /// @note OASIS-specific
    void addConfigItem( const QString & configName, bool value );
    /// @note OASIS-specific
    void addConfigItem( const QString & configName, int value );
    /// @note OASIS-specific
    void addConfigItem( const QString & configName, double value );
    /// @note OASIS-specific
    void addConfigItem( const QString & configName, long value );
    /// @note OASIS-specific
    void addConfigItem( const QString & configName, short value );

    // TODO addConfigItem for datetime and base64Binary

    /**
     * @brief Adds a text span as nodes of the current element.
     *
     * Unlike KoXmlWriter::addTextNode it handles tabulations, linebreaks,
     * and multiple spaces by using the appropriate OASIS tags.
     *
     * @param text the text to write
     *
     * @note OASIS-specific
     */
    void addTextSpan( const QString& text );
    /**
     * Overloaded version of addTextSpan which takes an additional tabCache map.
     * @param text the text to write
     * @param tabCache optional map allowing to find a tab for a given character index
     * @note OASIS-specific
     */
    void addTextSpan( const QString& text, const QMap<int, int>& tabCache );

    /**
     * @return the current indentation level.
     * Useful when creating a sub-KoXmlWriter (see addCompleteElement)
     */
    int indentLevel() const { return m_tags.size() + m_baseIndentLevel; }

private:
    struct Tag {
        Tag( const char* t = 0, bool ind = true )
            : tagName( t ), hasChildren( false ), lastChildIsText( false ),
              openingTagClosed( false ), indentInside( ind ) {}
        const char* tagName;
        bool hasChildren; ///< element or text children
        bool lastChildIsText; ///< last child is a text node
        bool openingTagClosed; ///< true once the '\>' in \<tag a="b"\> is written out
        bool indentInside; ///< whether to indent the contents of this tag
    };

    /// Write out \n followed by the number of spaces required.
    void writeIndent();

    // writeCString is much faster than writeString.
    // Try to use it as much as possible, especially with constants.
    void writeString( const QString& str );

    // unused and possibly incorrect if length != size
    //inline void writeCString( const QCString& cstr ) {
    //    m_dev->write( cstr.data(), cstr.size() - 1 );
    //}

    inline void writeCString( const char* cstr ) {
        m_dev->write( cstr, qstrlen( cstr ) );
    }
    inline void writeChar( char c ) {
        m_dev->putChar( c );
    }
    inline void closeStartElement( Tag& tag ) {
        if ( !tag.openingTagClosed ) {
            tag.openingTagClosed = true;
            writeChar( '>' );
        }
    }
    char* escapeForXML( const char* source, int length ) const;
    bool prepareForChild();
    void prepareForTextNode();
    void init();

    QIODevice* m_dev;
    QStack<Tag> m_tags;
    int m_baseIndentLevel;

    class Private;
    Private *d;

    char* m_indentBuffer; // maybe make it static, but then it needs a KStaticDeleter,
                          // and would eat 1K all the time... Maybe refcount it :)
    char* m_escapeBuffer; // can't really be static if we want to be thread-safe
    static const int s_escapeBufferLen = 10000;

    KoXmlWriter( const KoXmlWriter & ); // forbidden
    KoXmlWriter& operator=( const KoXmlWriter & ); // forbidden
};

#endif /* XMLWRITER_H */

