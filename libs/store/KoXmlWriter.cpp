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

#include "KoXmlWriter.h"

#include <kglobal.h> // kMin
#include <kdebug.h>
#include <QIODevice>
#include <QByteArray>
#include <float.h>

static const int s_indentBufferLength = 100;

KoXmlWriter::KoXmlWriter( QIODevice* dev, int indentLevel )
    : m_dev( dev ), m_baseIndentLevel( indentLevel )
{
    init();
}

void KoXmlWriter::init()
{
    m_indentBuffer = new char[ s_indentBufferLength ];
    memset( m_indentBuffer, ' ', s_indentBufferLength );
    *m_indentBuffer = '\n'; // write newline before indentation, in one go

    m_escapeBuffer = new char[s_escapeBufferLen];
    if ( !m_dev->isOpen() )
        m_dev->open( QIODevice::WriteOnly );
}

KoXmlWriter::~KoXmlWriter()
{
    delete[] m_indentBuffer;
    delete[] m_escapeBuffer;
}

void KoXmlWriter::startDocument( const char* rootElemName, const char* publicId, const char* systemId )
{
    Q_ASSERT( m_tags.isEmpty() );
    writeCString( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
    // There isn't much point in a doctype if there's no DTD to refer to
    // (I'm told that files that are validated by a RelaxNG schema cannot refer to the schema)
    if ( publicId ) {
        writeCString( "<!DOCTYPE " );
        writeCString( rootElemName );
        writeCString( " PUBLIC \"" );
        writeCString( publicId );
        writeCString( "\" \"" );
        writeCString( systemId );
        writeCString( "\"" );
        writeCString( ">\n" );
    }
}

void KoXmlWriter::endDocument()
{
    // just to do exactly like QDom does (newline at end of file).
    writeChar( '\n' );
    Q_ASSERT( m_tags.isEmpty() );
}

// returns the value of indentInside of the parent
bool KoXmlWriter::prepareForChild()
{
    if ( !m_tags.isEmpty() ) {
        Tag& parent = m_tags.top();
        if ( !parent.hasChildren ) {
            closeStartElement( parent );
            parent.hasChildren = true;
            parent.lastChildIsText = false;
        }
        if ( parent.indentInside ) {
            writeIndent();
        }
        return parent.indentInside;
    }
    return true;
}

void KoXmlWriter::prepareForTextNode()
{
    Tag& parent = m_tags.top();
    if ( !parent.hasChildren ) {
        closeStartElement( parent );
        parent.hasChildren = true;
        parent.lastChildIsText = true;
    }
}

void KoXmlWriter::startElement( const char* tagName, bool indentInside )
{
    Q_ASSERT( tagName != 0 );

    // Tell parent that it has children
    bool parentIndent = prepareForChild();

    m_tags.push( Tag( tagName, parentIndent && indentInside ) );
    writeChar( '<' );
    writeCString( tagName );
    //kDebug() << k_funcinfo << tagName << endl;
}

void KoXmlWriter::addCompleteElement( const char* cstr )
{
    prepareForChild();
    writeCString( cstr );
}


void KoXmlWriter::addCompleteElement( QIODevice* indev )
{
    prepareForChild();
    bool openOk = indev->open( QIODevice::ReadOnly );
    Q_ASSERT( openOk );
    if ( !openOk )
        return;
    static const int MAX_CHUNK_SIZE = 8*1024; // 8 KB
    QByteArray buffer;
    buffer.resize(MAX_CHUNK_SIZE);
    while ( !indev->atEnd() ) {
        qint64 len = indev->read( buffer.data(), buffer.size() );
        if ( len <= 0 ) // e.g. on error
            break;
        m_dev->write( buffer.data(), len );
    }
}

void KoXmlWriter::endElement()
{
    if ( m_tags.isEmpty() )
        kWarning() << "Ouch, endElement() was called more times than startElement(). "
            "The generated XML will be invalid! "
            "Please report this bug (by saving the document to another format...)" << endl;

    Tag tag = m_tags.pop();
    //kDebug() << k_funcinfo << " tagName=" << tag.tagName << " hasChildren=" << tag.hasChildren << endl;
    if ( !tag.hasChildren ) {
        writeCString( "/>" );
    }
    else {
        if ( tag.indentInside && !tag.lastChildIsText ) {
            writeIndent();
        }
        writeCString( "</" );
        Q_ASSERT( tag.tagName != 0 );
        writeCString( tag.tagName );
        writeChar( '>' );
    }
}

void KoXmlWriter::addTextNode( const QByteArray& cstr )
{
    // Same as the const char* version below, but here we know the size
    prepareForTextNode();
    char* escaped = escapeForXML( cstr.constData(), cstr.size() );
    writeCString( escaped );
    if(escaped != m_escapeBuffer)
        delete[] escaped;
}

void KoXmlWriter::addTextNode( const char* cstr )
{
    prepareForTextNode();
    char* escaped = escapeForXML( cstr, -1 );
    writeCString( escaped );
    if(escaped != m_escapeBuffer)
        delete[] escaped;
}

void KoXmlWriter::addProcessingInstruction( const char* cstr )
{
    prepareForTextNode();
    writeCString( "<?" );
    addTextNode( cstr );
    writeCString( "?>");
}

void KoXmlWriter::addAttribute( const char* attrName, const QByteArray& value )
{
    // Same as the const char* one, but here we know the size
    writeChar( ' ' );
    writeCString( attrName );
    writeCString("=\"");
    char* escaped = escapeForXML( value.constData(), value.size() );
    writeCString( escaped );
    if(escaped != m_escapeBuffer)
        delete[] escaped;
    writeChar( '"' );
}

void KoXmlWriter::addAttribute( const char* attrName, const char* value )
{
    writeChar( ' ' );
    writeCString( attrName );
    writeCString("=\"");
    char* escaped = escapeForXML( value, -1 );
    writeCString( escaped );
    if(escaped != m_escapeBuffer)
        delete[] escaped;
    writeChar( '"' );
}

void KoXmlWriter::addAttribute( const char* attrName, double value )
{
    QByteArray str;
    str.setNum( value, 'g', DBL_DIG );
    addAttribute( attrName, str.data() );
}

void KoXmlWriter::addAttributePt( const char* attrName, double value )
{
    QByteArray str;
    str.setNum( value, 'g', DBL_DIG );
    str += "pt";
    addAttribute( attrName, str.data() );
}

void KoXmlWriter::writeIndent()
{
    // +1 because of the leading '\n'
    m_dev->write( m_indentBuffer, qMin( indentLevel() + 1,
                                        s_indentBufferLength ) );
}

void KoXmlWriter::writeString( const QString& str )
{
    // cachegrind says .utf8() is where most of the time is spent
    const QByteArray cstr = str.toUtf8();
    m_dev->write( cstr );
}

// In case of a reallocation (ret value != m_buffer), the caller owns the return value,
// it must delete it (with [])
char* KoXmlWriter::escapeForXML( const char* source, int length = -1 ) const
{
    // we're going to be pessimistic on char length; so lets make the outputLength less
    // the amount one char can take: 6
    char* destBoundary = m_escapeBuffer + s_escapeBufferLen - 6;
    char* destination = m_escapeBuffer;
    char* output = m_escapeBuffer;
    const char* src = source; // src moves, source remains
    for ( ;; ) {
        if(destination >= destBoundary) {
            // When we come to realize that our escaped string is going to
            // be bigger than the escape buffer (this shouldn't happen very often...),
            // we drop the idea of using it, and we allocate a bigger buffer.
            // Note that this if() can only be hit once per call to the method.
            if ( length == -1 )
                length = qstrlen( source ); // expensive...
            uint newLength = length * 6 + 1; // worst case. 6 is due to &quot; and &apos;
            char* buffer = new char[ newLength ];
            destBoundary = buffer + newLength;
            uint amountOfCharsAlreadyCopied = destination - m_escapeBuffer;
            memcpy( buffer, m_escapeBuffer, amountOfCharsAlreadyCopied );
            output = buffer;
            destination = buffer + amountOfCharsAlreadyCopied;
        }
        switch( *src ) {
        case 60: // <
            memcpy( destination, "&lt;", 4 );
            destination += 4;
            break;
        case 62: // >
            memcpy( destination, "&gt;", 4 );
            destination += 4;
            break;
        case 34: // "
            memcpy( destination, "&quot;", 6 );
            destination += 6;
            break;
#if 0 // needed?
        case 39: // '
            memcpy( destination, "&apos;", 6 );
            destination += 6;
            break;
#endif
        case 38: // &
            memcpy( destination, "&amp;", 5 );
            destination += 5;
            break;
        case 0:
            *destination = '\0';
            return output;
        default:
            *destination++ = *src++;
            continue;
        }
        ++src;
    }
    // NOTREACHED (see case 0)
    return output;
}

void KoXmlWriter::addManifestEntry( const QString& fullPath, const QString& mediaType )
{
    startElement( "manifest:file-entry" );
    addAttribute( "manifest:media-type", mediaType );
    addAttribute( "manifest:full-path", fullPath );
    endElement();
}

void KoXmlWriter::addConfigItem( const QString & configName, const QString& value )
{
    startElement( "config:config-item" );
    addAttribute( "config:name", configName );
    addAttribute( "config:type",  "string" );
    addTextNode( value );
    endElement();
}

void KoXmlWriter::addConfigItem( const QString & configName, bool value )
{
    startElement( "config:config-item" );
    addAttribute( "config:name", configName );
    addAttribute( "config:type",  "boolean" );
    addTextNode( value ? "true" : "false" );
    endElement();
}

void KoXmlWriter::addConfigItem( const QString & configName, int value )
{
    startElement( "config:config-item" );
    addAttribute( "config:name", configName );
    addAttribute( "config:type",  "int");
    addTextNode(QString::number( value ) );
    endElement();
}

void KoXmlWriter::addConfigItem( const QString & configName, double value )
{
    startElement( "config:config-item" );
    addAttribute( "config:name", configName );
    addAttribute( "config:type", "double" );
    addTextNode( QString::number( value ) );
    endElement();
}

void KoXmlWriter::addConfigItem( const QString & configName, long value )
{
    startElement( "config:config-item" );
    addAttribute( "config:name", configName );
    addAttribute( "config:type", "long" );
    addTextNode( QString::number( value ) );
    endElement();
}

void KoXmlWriter::addConfigItem( const QString & configName, short value )
{
    startElement( "config:config-item" );
    addAttribute( "config:name", configName );
    addAttribute( "config:type", "short" );
    addTextNode( QString::number( value ) );
    endElement();
}

void KoXmlWriter::addTextSpan( const QString& text )
{
    QMap<int, int> tabCache;
    addTextSpan( text, tabCache );
}

void KoXmlWriter::addTextSpan( const QString& text, const QMap<int, int>& tabCache )
{
    int len = text.length();
    int nrSpaces = 0; // number of consecutive spaces
    QString str;
    str.reserve( len );
    // Accumulate chars either in str or in nrSpaces (for spaces).
    // Flush str when writing a subelement (for spaces or for another reason)
    // Flush nrSpaces when encountering two or more consecutive spaces
    for ( int i = 0; i < len ; ++i ) {
        QChar ch = text[i];
        if ( ch != ' ' ) {
            if ( nrSpaces > 0 ) {
                // For the first space we use ' '.
                // "it is good practice to use (text:s) for the second and all following SPACE characters in a sequence."
                str += ' ';
                --nrSpaces;
                if ( nrSpaces > 0 ) { // there are more spaces
                    if ( !str.isEmpty() )
                        addTextNode( str );
                    str = QString::null;
                    startElement( "text:s" );
                    if ( nrSpaces > 1 ) // it's 1 by default
                        addAttribute( "text:c", nrSpaces );
                    endElement();
                }
            }
            nrSpaces = 0;
        }
        switch ( ch.unicode() ) {
        case '\t':
            if ( !str.isEmpty() )
                addTextNode( str );
            str = QString::null;
            startElement( "text:tab" );
            if ( tabCache.contains( i ) )
                addAttribute( "text:tab-ref", tabCache[i] + 1 );
            endElement();
            break;
        case '\n':
            if ( !str.isEmpty() )
                addTextNode( str );
            str = QString::null;
            startElement( "text:line-break" );
            endElement();
            break;
        case ' ':
            ++nrSpaces;
            break;
        default:
            str += text[i];
            break;
        }
    }
    // either we still have text in str or we have spaces in nrSpaces
    if ( nrSpaces > 0 ) {
        str += ' ';
        --nrSpaces;
    }
    if ( !str.isEmpty() ) {
        addTextNode( str );
    }
    if ( nrSpaces > 0 ) { // there are more spaces
        startElement( "text:s" );
        if ( nrSpaces > 1 ) // it's 1 by default
            addAttribute( "text:c", nrSpaces );
        endElement();
    }
}
