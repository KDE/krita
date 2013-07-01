/* This file is part of the KDE project

   Copyright (C) 2013 Inge Wallin <inge@lysator.liu.se>

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

#ifndef KOXMLSTREAMREADER_H
#define KOXMLSTREAMREADER_H


#include "KoXmlStreamReader.h"

#include <QXmlStreamReader>
#include <QStringRef>
#include <QVector>
#include <QSharedData>

#include "koodf_export.h"


class QByteArray;
class QString;
class QIODevice;

class KoXmlStreamAttributes;


/**
 * @brief An XML stream reader based on QXmlStreamReader and with namespace handling better suited to use for ODF in Calligra.
 *
 * Opendocument uses an XML encoding which makes heavy use of
 * namespaces. So normally you would want to compare the qualified
 * name when accessing tagnames and attributes.
 * 
 * However, in QXmlStreamReader you have to either make an explicit
 * comparison with the namespace URI for every element and attribute
 * or risk that documents that use the correct namespaces but not the
 * normal namespace prefixes are wrongly interpreted.  This is because
 * the prefix (e.g. "fo" in "fo:border-width") is declared at the
 * beginning of the document using a namespace declaration attribute
 * such as: xmlns:fo="http://www.w3.org/1999/XSL/Format". In this case
 * xmlns:fo could just as well be xmlns:xxx which makes the expected
 * fo:border-width become xxx:border-width in the rest of this
 * document.
 *
 * However, it is extremely rare to find document that uses such
 * non-standard namespace prefixes. This gives us the opportunity to
 * optimize for the common case, which is exactly what
 * KoXmlStreamReader does.
 *
 * The way to use this class is to tell it which namespaces and
 * prefixes that you expect before you open the XML stream. Then it
 * checks if the namespaces and prefixes in the document are the same
 * as the expected ones.  If they are in fact the same, the document
 * is pronounced "sound", and for the rest of the processing you can
 * use the qualified name with the expected prefix ("fo:border-width")
 * with the maximum performance.
 *
 * If the namespace(s) in the document are the expected ones but the
 * prefix(es) are not, you can still compare the qualified name to
 * your expected ones.  But in this case the document is deemed
 * "unsound" and for every access to attributes or calls to
 * qualifiedName(), KoXmlStreamReader will rewrite the actual name in
 * the document to become what you expect.  The functions
 * namespaceUri() and name() are not affected, only the prefixes.
 */
class KOODF_EXPORT KoXmlStreamReader : public QXmlStreamReader
{
    friend class KoXmlStreamAttribute;
    friend class KoXmlStreamAttributes;

public:
    KoXmlStreamReader();
    KoXmlStreamReader(QIODevice *device);
    KoXmlStreamReader(const QByteArray &data);
    KoXmlStreamReader(const QString &data);
    KoXmlStreamReader(const char *data);

    ~KoXmlStreamReader();

    void clear();

    void addExpectedNamespace(const QString &prefix, const QString &namespaceUri);
    void addExtraNamespace(const QString &prefix, const QString &namespaceUri);

    // --------------------------------
    // Reimplemented from QXmlStreamReader

    QStringRef prefix() const;
    QStringRef qualifiedName() const;
    void setDevice(QIODevice *device);
    KoXmlStreamAttributes attributes() const;

private:
    // No copying
    KoXmlStreamReader(KoXmlStreamReader &other);
    KoXmlStreamReader &operator=(KoXmlStreamReader &other);

    // Only for friend classes KoXmlStreamAttributes and KoXmlStreamAttribute.
    bool isSound() const;

    class Private;
    Private * const d;
};


/**
 * @brief KoXmlStreamAttribute is a source-compatible replacement for QXmlStreamAttribute.
 *
 * In addition to the API from QXmlStreamAttribute, it offers the same
 * advantages that KoXmlStreamReader does over QXmlStreamReader: when
 * asked for the qualified name of an attribute it will return the
 * expected one even if the prefix declared in the namespace
 * declaration of the document is different.
 *
 * @see KoXmlStreamReader
 */
class KoXmlStreamAttribute
{
    friend class QVector<KoXmlStreamAttribute>;       // For the default constructor
    friend class KoXmlStreamAttributes;               // For the normal constructor
    friend class KoXmlStreamReader;
 public:
    ~KoXmlStreamAttribute();

    // API taken from QXmlStreamAttribute
    bool       isDefault() const;
    QStringRef name() const;
    QStringRef namespaceUri() const;
    QStringRef prefix() const;
    QStringRef qualifiedName() const;
    QStringRef value() const;

    bool operator==(const KoXmlStreamAttribute &other) const;
    bool operator!=(const KoXmlStreamAttribute &other) const;
    KoXmlStreamAttribute &operator=(const KoXmlStreamAttribute &other);

 private:
    // Only for friend classes.
    KoXmlStreamAttribute();
    KoXmlStreamAttribute(const KoXmlStreamAttribute &other);
    KoXmlStreamAttribute(const QXmlStreamAttribute *attr, const KoXmlStreamReader *reader);

    class Private;
    Private * const d;
};


/**
 * @brief KoXmlStreamAttributes is a mostly source-compatible replacement for QXmlStreamAttributes.
 *
 * All the convenience functions of KoXmlStreamAttributes work exactly
 * like the counterparts of QXmlStreamAttributes but they give the
 * expected prefix for the registered expected namespaces.
 *
 * Not all functions from QVector are implemented but the ones that
 * make sense for this read-only class are. This class can only be
 * used in connection with KoXmlStreamReader.
 *
 * @see KoXmlStreamReader
 */
class KoXmlStreamAttributes
{
    friend class KoXmlStreamReader;

 public:
    typedef const KoXmlStreamAttribute *const_iterator;

    KoXmlStreamAttributes(const KoXmlStreamAttributes &other);
    ~KoXmlStreamAttributes();

    KoXmlStreamAttributes &operator=(const KoXmlStreamAttributes &other);

    // Relevant parts of the QVector API
    const KoXmlStreamAttribute &at(int i) const;
    int size() const;
    KoXmlStreamAttribute value(int i) const;
    const KoXmlStreamAttribute &operator[](int i) const;
    const_iterator begin() const;
    const_iterator end() const;

    // Convenience functions taken from QXmlStreamAttributes API
    void        append(const QString &namespaceUri, const QString &name, const QString &value);
    void        append(const QXmlStreamAttribute &attribute);
    void        append(const QString &qualifiedName, const QString &value);
    bool        hasAttribute(const QString &qualifiedName) const;
    bool        hasAttribute(const QLatin1String &qualifiedName) const;
    bool        hasAttribute ( const QString & namespaceUri, const QString & name ) const;
    QStringRef  value ( const QString & namespaceUri, const QString & name ) const;
    QStringRef  value ( const QString & namespaceUri, const QLatin1String & name ) const;
    QStringRef  value ( const QLatin1String & namespaceUri, const QLatin1String & name ) const;
    QStringRef  value(const QString &qualifiedName) const;
    QStringRef  value(const QLatin1String &qualifiedName) const;

 private:
    // Only available from friend class KoXmlStreamReader.
    KoXmlStreamAttributes(const KoXmlStreamReader *r, const QXmlStreamAttributes &qAttrs);

    // This class is implicitly shared.
    class Private;
    QSharedDataPointer<Private> d;
};


void prepareForOdf(KoXmlStreamReader &reader);


#endif /* KOGENCHANGES_H */
