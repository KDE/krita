/* This file is part of the KDE project
   Copyright (C) 2005-2006 Ariya Hidayat <ariya@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#include "KoXmlReader.h"
#include "KoXmlNS.h"

/*
  This is a memory-efficient DOM implementation for KOffice. See the API
  documentation for details.

  IMPORTANT !

  * When you change this stuff, make sure it DOES NOT BREAK the test suite.
    Build tests/koxmlreadertest.cpp and verify it. Many sleepless nights
    have been sacrificed for this piece of code, do not let those precious
    hours wasted!

  * Run koxmlreadertest.cpp WITH Valgrind and make sure NO illegal
    memory read/write and any type of leak occurs. If you are not familiar
    with Valgrind then RTFM first and come back again later on.

  * The public API shall remain as compatible as QDom.

  * All QDom-compatible methods should behave the same. All QDom-compatible
    functions should return the same result. In case of doubt, run
    koxmlreadertest.cpp but uncomment KOXML_USE_QDOM in koxmlreader.h
    so that the tests are performed with standard QDom.

  Some differences compared to QDom:

  - DOM tree in KoXmlDocument is read-only, you can not modify it. This is
    sufficient for KOffice since the tree is only accessed when loading
    a document to the application. For saving the document to XML file,
    use KoXmlWriter.

  - Because the dynamic loading and unloading, you have to use the
    nodes (and therefore also elements) carefully since the whole API
 (just like QDom) is reference-based, not pointer-based. If the
 parent node is unloaded from memory, the reference is not valid
 anymore and may give unpredictable result.
 The easiest way: use the node/element in very short time only.

  - Comment node (like QDomComment) is not implemented as comments are
    simply ignored.

  - DTD, entity and entity reference are not handled. Thus, the associated
    nodes (like QDomDocumentType, QDomEntity, QDomEntityReference) are also
    not implemented.

  - Attribute mapping node is not implemented. But of course, functions to
    query attributes of an element are available.


 */

#include <QTextCodec>
#include <QTextDecoder>

#ifndef KOXML_USE_QDOM

#include <qxml.h>
#include <qdom.h>
#include <QXmlStreamReader>
#include <QXmlStreamEntityResolver>

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QHash>
#include <QPair>
#include <QStringList>
#include <QVector>

/*
 Use more compact representation of in-memory nodes.

 Advantages: faster iteration, can facilitate real-time compression.
 Disadvantages: still buggy, eat slightly more memory.
*/
#define KOXML_COMPACT

/*
 Use real-time compression. Only works in conjuction with KOXML_COMPACT
 above because otherwise the non-compact layout will slow down everything.
*/
#define KOXML_COMPRESS


// prevent mistake, see above
#ifdef KOXML_COMPRESS
#ifndef KOXML_COMPACT
#error Please enable also KOXML_COMPACT
#endif
#endif

// this is used to quickly get namespaced attribute(s)
typedef QPair<QString, QString> KoXmlStringPair;

class KoQName {
public:
    QString nsURI;
    QString name;

    explicit KoQName(const QString& nsURI_, const QString& name_)
        : nsURI(nsURI_), name(name_) {}
    bool operator==(const KoQName& qname) const {
        // local name is more likely to differ, so compare that first
        return name == qname.name && nsURI == qname.nsURI;
    }
};

uint qHash(const KoQName& qname)
{
    // possibly add a faster hash function that only includes some trailing
    // part of the nsURI

    // in case of doubt, use this:
    // return qHash(qname.nsURI)^qHash(qname.name);
    return qHash(qname.nsURI)^qHash(qname.name);
}

// this simplistic hash is rather fast-and-furious. it works because
// likely there is very few namespaced attributes per element
static inline uint qHash(const KoXmlStringPair &p)
{
    return qHash(p.first[0].unicode()) ^ 0x1477;

    // in case of doubt, use this:
    // return qHash(p.first)^qHash(p.second);
}

static inline bool operator==(const KoXmlStringPair &a, const KoXmlStringPair &b)
{
    return a.second == b.second && a.first == b.first;
}

// Older versions of OpenOffice.org used different namespaces. This function
// does translate the old namespaces into the new ones.
static QString fixNamespace(const QString &nsURI)
{
    static QString office = QString::fromLatin1("http://openoffice.org/2000/office");
    static QString text = QString::fromLatin1("http://openoffice.org/2000/text");
    static QString style = QString::fromLatin1("http://openoffice.org/2000/style");
    static QString fo = QString::fromLatin1("http://www.w3.org/1999/XSL/Format");
    static QString table = QString::fromLatin1("http://openoffice.org/2000/table");
    static QString drawing = QString::fromLatin1("http://openoffice.org/2000/drawing");
    static QString datastyle = QString::fromLatin1("http://openoffice.org/2000/datastyle");
    static QString svg = QString::fromLatin1("http://www.w3.org/2000/svg");
    static QString chart = QString::fromLatin1("http://openoffice.org/2000/chart");
    static QString dr3d = QString::fromLatin1("http://openoffice.org/2000/dr3d");
    static QString form = QString::fromLatin1("http://openoffice.org/2000/form");
    static QString script = QString::fromLatin1("http://openoffice.org/2000/script");
    static QString meta = QString::fromLatin1("http://openoffice.org/2000/meta");
    static QString config = QString::fromLatin1("http://openoffice.org/2001/config");
    static QString pres = QString::fromLatin1("http://openoffice.org/2000/presentation");
    static QString manifest = QString::fromLatin1("http://openoffice.org/2001/manifest");
    if (nsURI == text)
        return KoXmlNS::text;
    if (nsURI == style)
        return KoXmlNS::style;
    if (nsURI == office)
        return KoXmlNS::office;
    if (nsURI == fo)
        return KoXmlNS::fo;
    if (nsURI == table)
        return KoXmlNS::table;
    if (nsURI == drawing)
        return KoXmlNS::draw;
    if (nsURI == datastyle)
        return KoXmlNS::number;
    if (nsURI == svg)
        return KoXmlNS::svg;
    if (nsURI == chart)
        return KoXmlNS::chart;
    if (nsURI == dr3d)
        return KoXmlNS::dr3d;
    if (nsURI == form)
        return KoXmlNS::form;
    if (nsURI == script)
        return KoXmlNS::script;
    if (nsURI == meta)
        return KoXmlNS::meta;
    if (nsURI == config)
        return KoXmlNS::config;
    if (nsURI == pres)
        return KoXmlNS::presentation;
    if (nsURI == manifest)
        return KoXmlNS::manifest;
    return nsURI;
}

// ==================================================================
//
//         KoXmlPackedItem
//
// ==================================================================

// 12 bytes on most system 32 bit systems, 16 bytes on 64 bit systems
class KoXmlPackedItem
{
public:
bool attr: 1;
KoXmlNode::NodeType type: 3;

#ifdef KOXML_COMPACT
quint32 childStart: 28;
#else
unsigned depth: 28;
#endif

    unsigned qnameIndex;
    QString value;

    // it is important NOT to have a copy constructor, so that growth is optimal
    // see http://doc.trolltech.com/4.2/containers.html#growth-strategies
#if 0
    KoXmlPackedItem(): attr(false), type(KoXmlNode::NullNode), childStart(0), depth(0) {}
#endif
};

Q_DECLARE_TYPEINFO(KoXmlPackedItem, Q_MOVABLE_TYPE);

#ifdef KOXML_COMPRESS
static QDataStream& operator<<(QDataStream& s, const KoXmlPackedItem& item)
{
    quint8 flag = item.attr ? 1 : 0;

    s << flag;
    s << (quint8) item.type;
    s << item.childStart;
    s << item.qnameIndex;
    s << item.value;

    return s;
}

static QDataStream& operator>>(QDataStream& s, KoXmlPackedItem& item)
{
    quint8 flag;
    quint8 type;
    quint32 child;
    QString value;

    s >> flag;
    s >> type;
    s >> child;
    s >> item.qnameIndex;
    s >> value;

    item.attr = (flag != 0);
    item.type = (KoXmlNode::NodeType) type;
    item.childStart = child;
    item.value = value;

    return s;
}
#endif

#ifdef KOXML_COMPRESS

// ==================================================================
//
//         KoXmlVector
//
// ==================================================================


// similar to QVector, but using LZF compression to save memory space
// this class is however not reentrant

// comment it to test this class without the compression
#define KOXMLVECTOR_USE_LZF

// when number of buffered items reach this, compression will start
// small value will give better memory usage at the cost of speed
// bigger value will be better in term of speed, but use more memory
#define ITEMS_FULL  (1*256)

// LZF stuff is wrapper in KoLZF
#ifdef KOXML_COMPRESS
#ifdef KOXMLVECTOR_USE_LZF

#define HASH_LOG  12
#define HASH_SIZE (1<< HASH_LOG)
#define HASH_MASK  (HASH_SIZE-1)

#define UPDATE_HASH(v,p) { v = *((quint16*)p); v ^= *((quint16*)(p+1))^(v>>(16-HASH_LOG)); }

#define MAX_COPY       32
#define MAX_LEN       264  /* 256 + 8 */
#define MAX_DISTANCE 8192

// Lossless compression using LZF algorithm, this is faster on modern CPU than
// the original implementation in http://liblzf.plan9.de/
static int lzff_compress(const void* input, int length, void* output, int maxout)
{
    Q_UNUSED(maxout);

    const quint8* ip = (const quint8*) input;
    const quint8* ip_limit = ip + length - MAX_COPY - 4;
    quint8* op = (quint8*) output;

    const quint8* htab[HASH_SIZE];
    const quint8** hslot;
    quint32 hval;

    quint8* ref;
    qint32 copy;
    qint32 len;
    qint32 distance;
    quint8* anchor;

    /* initializes hash table */
    for (hslot = htab; hslot < htab + HASH_SIZE; hslot++)
        *hslot = ip;

    /* we start with literal copy */
    copy = 0;
    *op++ = MAX_COPY - 1;

    /* main loop */
    while (ip < ip_limit) {
        /* find potential match */
        UPDATE_HASH(hval, ip);
        hslot = htab + (hval & HASH_MASK);
        ref = (quint8*) * hslot;

        /* update hash table */
        *hslot = ip;

        /* find itself? then it's no match */
        if (ip == ref)
            goto literal;

        /* is this a match? check the first 2 bytes */
        if (*((quint16*)ref) != *((quint16*)ip))
            goto literal;

        /* now check the 3rd byte */
        if (ref[2] != ip[2])
            goto literal;

        /* calculate distance to the match */
        distance = ip - ref;

        /* skip if too far away */
        if (distance >= MAX_DISTANCE)
            goto literal;

        /* here we have 3-byte matches */
        anchor = (quint8*)ip;
        len = 3;
        ref += 3;
        ip += 3;

        /* now we have to check how long the match is */
        if (ip < ip_limit - MAX_LEN) {
            while (len < MAX_LEN - 8) {
                /* unroll 8 times */
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                len += 8;
            }
            ip--;
        }
        len = ip - anchor;

        /* just before the last non-matching byte */
        ip = anchor + len;

        /* if we have copied something, adjust the copy count */
        if (copy) {
            /* copy is biased, '0' means 1 byte copy */
            anchor = anchor - copy - 1;
            *(op - copy - 1) = copy - 1;
            copy = 0;
        } else
            /* back, to overwrite the copy count */
            op--;

        /* length is biased, '1' means a match of 3 bytes */
        len -= 2;

        /* distance is also biased */
        distance--;

        /* encode the match */
        if (len < 7)
            *op++ = (len << 5) + (distance >> 8);
        else {
            *op++ = (7 << 5) + (distance >> 8);
            *op++ = len - 7;
        }
        *op++ = (distance & 255);

        /* assuming next will be literal copy */
        *op++ = MAX_COPY - 1;

        /* update the hash at match boundary */
        --ip;
        UPDATE_HASH(hval, ip);
        htab[hval & HASH_MASK] = ip;
        ip++;

        continue;

    literal:
        *op++ = *ip++;
        copy++;
        if (copy >= MAX_COPY) {
            copy = 0;
            *op++ = MAX_COPY - 1;
        }
    }

    /* left-over as literal copy */
    ip_limit = (const quint8*)input + length;
    while (ip < ip_limit) {
        *op++ = *ip++;
        copy++;
        if (copy == MAX_COPY) {
            copy = 0;
            *op++ = MAX_COPY - 1;
        }
    }

    /* if we have copied something, adjust the copy length */
    if (copy)
        *(op - copy - 1) = copy - 1;
    else
        op--;

    return op - (quint8*)output;
}

static int lzff_decompress(const void* input, int length, void* output, int maxout)
{
    const quint8* ip = (const quint8*) input;
    const quint8* ip_limit  = ip + length - 1;
    quint8* op = (quint8*) output;
    quint8* op_limit = op + maxout;
    quint8* ref;

    while (ip < ip_limit) {
        quint32 ctrl = (*ip) + 1;
        quint32 ofs = ((*ip) & 31) << 8;
        quint32 len = (*ip++) >> 5;

        if (ctrl < 33) {
            /* literal copy */
            if (op + ctrl > op_limit)
                return 0;

            /* crazy unrolling */
            if (ctrl) {
                *op++ = *ip++;
                ctrl--;

                if (ctrl) {
                    *op++ = *ip++;
                    ctrl--;

                    if (ctrl) {
                        *op++ = *ip++;
                        ctrl--;

                        for (;ctrl; ctrl--)
                            *op++ = *ip++;
                    }
                }
            }
        } else {
            /* back reference */
            len--;
            ref = op - ofs;
            ref--;

            if (len == 7 - 1)
                len += *ip++;

            ref -= *ip++;

            if (op + len + 3 > op_limit)
                return 0;

            if (ref < (quint8 *)output)
                return 0;

            *op++ = *ref++;
            *op++ = *ref++;
            *op++ = *ref++;
            if (len)
                for (; len; --len)
                    *op++ = *ref++;
        }
    }

    return op - (quint8*)output;
}

class KoLZF
{
public:
    static QByteArray compress(const QByteArray&);
    static void decompress(const QByteArray&, QByteArray&);
};

QByteArray KoLZF::compress(const QByteArray& input)
{
    const void* const in_data = (const void*) input.constData();
    unsigned int in_len = (unsigned int)input.size();

    QByteArray output;
    output.resize(in_len + 4 + 1);

    // we use 4 bytes to store uncompressed length
    // and 1 extra byte as flag (0=uncompressed, 1=compressed)
    output[0] = in_len & 255;
    output[1] = (in_len >> 8) & 255;
    output[2] = (in_len >> 16) & 255;
    output[3] = (in_len >> 24) & 255;
    output[4] = 1;

    unsigned int out_len = in_len - 1;
    unsigned char* out_data = (unsigned char*) output.data() + 5;

    unsigned int len = lzff_compress(in_data, in_len, out_data, out_len);
    out_len = len;

    if ((len > out_len) || (len == 0)) {
        // output buffer is too small, likely because the data can't
        // be compressed. so here just copy without compression
        out_len = in_len;
        output.insert(5, input);

        // flag must indicate "uncompressed block"
        output[4] = 0;
    }

    // minimize memory
    output.resize(out_len + 4 + 1);
    output.squeeze();

    return output;
}

// will not squeeze output
void KoLZF::decompress(const QByteArray& input, QByteArray& output)
{
    // read out first how big is the uncompressed size
    unsigned int unpack_size = 0;
    unpack_size |= ((quint8)input[0]);
    unpack_size |= ((quint8)input[1]) << 8;
    unpack_size |= ((quint8)input[2]) << 16;
    unpack_size |= ((quint8)input[3]) << 24;

    // prepare the output
    output.reserve(unpack_size);

    // compression flag
    quint8 flag = (quint8)input[4];

    // prepare for lzf
    const void* const in_data = (const void*)(input.constData() + 5);
    unsigned int in_len = (unsigned int)input.size() - 5;
    unsigned char* out_data = (unsigned char*) output.data();
    unsigned int out_len = (unsigned int)unpack_size;

    if (flag == 0)
        memcpy(output.data(), in_data, in_len);
    else {
        unsigned int len = lzff_decompress(in_data, in_len, out_data, out_len);
        output.resize(len);
        output.squeeze();
    }
}


#endif
#endif

template <typename T>
class KoXmlVector
{
private:
    unsigned totalItems;
    QVector<unsigned> startIndex;
    QVector<QByteArray> blocks;

    unsigned bufferStartIndex;
    QVector<T> bufferItems;
    QByteArray bufferData;

protected:
    // fetch given item index to the buffer
    // will INVALIDATE all references to the buffer
    void fetchItem(unsigned index) {
        // already in the buffer ?
        if (index >= bufferStartIndex)
            if (index - bufferStartIndex < (unsigned)bufferItems.count())
                return;

        // search in the stored blocks
        // TODO: binary search to speed up
        int loc = startIndex.count() - 1;
        for (int c = 0; c < startIndex.count() - 1; c++)
            if (index >= startIndex[c])
                if (index < startIndex[c+1]) {
                    loc = c;
                    break;
                }

        bufferStartIndex = startIndex[loc];
#ifdef KOXMLVECTOR_USE_LZF
        KoLZF::decompress(blocks[loc], bufferData);
#else
        bufferData = blocks[loc];
#endif
        QBuffer buffer(&bufferData);
        buffer.open(QIODevice::ReadOnly);
        QDataStream in(&buffer);
        bufferItems.clear();
        in >> bufferItems;
    }

    // store data in the buffer to main blocks
    void storeBuffer() {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        QDataStream out(&buffer);
        out << bufferItems;

        startIndex.append(bufferStartIndex);
#ifdef KOXMLVECTOR_USE_LZF
        blocks.append(KoLZF::compress(buffer.data()));
#else
        blocks.append(buffer.data());
#endif

        bufferStartIndex += bufferItems.count();
        bufferItems.clear();
    }

public:
    inline KoXmlVector(): totalItems(0), bufferStartIndex(0) {};

    void clear() {
        totalItems = 0;
        startIndex.clear();
        blocks.clear();

        bufferStartIndex = 0;
        bufferItems.clear();
        bufferData.reserve(1024*1024);
    }

    inline int count() const {
        return (int)totalItems;
    }
    inline int size() const {
        return (int)totalItems;
    }
    inline bool isEmpty() const {
        return totalItems == 0;
    }

    // append a new item
    // WARNING: use the return value as soon as possible
    // it may be invalid if another function is invoked
    T& newItem() {
        // buffer full?
        if (bufferItems.count() >= ITEMS_FULL - 1)
            storeBuffer();

        totalItems++;
        bufferItems.resize(bufferItems.count() + 1);
        return bufferItems[bufferItems.count()-1];
    }

    // WARNING: use the return value as soon as possible
    // it may be invalid if another function is invoked
    const T &operator[](int i) const {
        ((KoXmlVector*)this)->fetchItem((unsigned)i);
        return bufferItems[i - bufferStartIndex];
    }

    // optimize memory usage
    // will INVALIDATE all references to the buffer
    void squeeze() {
        storeBuffer();
    }

};

#endif

// ==================================================================
//
//         KoXmlPackedDocument
//
// ==================================================================

#ifdef KOXML_COMPRESS
typedef KoXmlVector<KoXmlPackedItem> KoXmlPackedGroup;
#else
typedef QVector<KoXmlPackedItem> KoXmlPackedGroup;
#endif

// growth strategy: increase every GROUP_GROW_SIZE items
// this will override standard QVector's growth strategy
#define GROUP_GROW_SHIFT 3
#define GROUP_GROW_SIZE (1 << GROUP_GROW_SHIFT)

class KoXmlPackedDocument
{
public:
    bool processNamespace;
#ifdef KOXML_COMPACT
    // map given depth to the list of items
    QHash<int, KoXmlPackedGroup> groups;
#else
    QVector<KoXmlPackedItem> items;
#endif

    QList<KoQName> qnameList;
    QString docType;

private:
    QHash<KoQName, unsigned> qnameHash;

    unsigned cacheQName(const QString& name, const QString& nsURI) {
        KoQName qname(nsURI, name);

        const unsigned ii = qnameHash.value(qname, (unsigned)-1);
        if (ii != (unsigned)-1)
            return ii;

        // not yet declared, so we add it
        unsigned i = qnameList.count();
        qnameList.append(qname);
        qnameHash.insert(qname, i);

        return i;
    }

    QHash<QString, unsigned> valueHash;
    QStringList valueList;

    QString cacheValue(const QString& value) {
        if (value.isEmpty())
            return 0;

        const unsigned& ii = valueHash[value];
        if (ii > 0)
            return valueList[ii];

        // not yet declared, so we add it
        unsigned i = valueList.count();
        valueList.append(value);
        valueHash.insert(value, i);

        return valueList[i];
    }

#ifdef KOXML_COMPACT
public:
    const KoXmlPackedItem& itemAt(unsigned depth, unsigned index) {
        const KoXmlPackedGroup& group = groups[depth];
        return group[index];
    }

    unsigned itemCount(unsigned depth) {
        const KoXmlPackedGroup& group = groups[depth];
        return group.count();
    }

    /*
       NOTE:
          Function clear, newItem, addElement, addAttribute, addText,
          addCData, addProcessing are all related. These are all necessary
          for stateful manipulation of the document. See also the calls
          to these function from parseDocument().

          The state itself is defined by the member variables
          currentDepth and the groups (see above).
     */

    unsigned currentDepth;

    KoXmlPackedItem& newItem(unsigned depth) {
        KoXmlPackedGroup& group = groups[depth];

#ifdef KOXML_COMPRESS
        KoXmlPackedItem& item = group.newItem();
#else
        // reserve up front
        if ((groups.size() % GROUP_GROW_SIZE) == 0)
            group.reserve(GROUP_GROW_SIZE * (1 + (groups.size() >> GROUP_GROW_SHIFT)));
        group.resize(group.count() + 1);

        KoXmlPackedItem& item = group[group.count()-1];
#endif

        // this is necessary, because intentionally we don't want to have
        // a constructor for KoXmlPackedItem
        item.attr = false;
        item.type = KoXmlNode::NullNode;
        item.qnameIndex = 0;
        item.childStart = itemCount(depth + 1);
        item.value.clear();

        return item;
    }

    void clear() {
        currentDepth = 0;
        qnameHash.clear();
        qnameList.clear();
        valueHash.clear();
        valueList.clear();
        groups.clear();
        docType.clear();

        // first node is root
        KoXmlPackedItem& rootItem = newItem(0);
        rootItem.type = KoXmlNode::DocumentNode;
    }

    void finish() {
        // won't be needed anymore
        qnameHash.clear();
        valueHash.clear();
        valueList.clear();

        // optimize, see documentation on QVector::squeeze
        for (int d = 0; d < groups.count(); d++) {
            KoXmlPackedGroup& group = groups[d];
            group.squeeze();
        }
    }

    // in case namespace processing, 'name' contains the prefix already
    void addElement(const QString& name, const QString& nsURI) {
        KoXmlPackedItem& item = newItem(currentDepth + 1);
        item.type = KoXmlNode::ElementNode;
        item.qnameIndex = cacheQName(name, nsURI);

        currentDepth++;
    }

    void closeElement() {
        currentDepth--;
    }

    void addDTD(const QString& dt) {
        docType = dt;
    }

    void addAttribute(const QString& name, const QString& nsURI, const QString& value) {
        KoXmlPackedItem& item = newItem(currentDepth + 1);
        item.attr = true;
        item.qnameIndex = cacheQName(name, nsURI);
        //item.value = cacheValue( value );
        item.value = value;
    }

    void addText(const QString& text) {
        KoXmlPackedItem& item = newItem(currentDepth + 1);
        item.type = KoXmlNode::TextNode;
        item.value = text;
    }

    void addCData(const QString& text) {
        KoXmlPackedItem& item = newItem(currentDepth + 1);
        item.type = KoXmlNode::CDATASectionNode;
        item.value = text;
    }

    void addProcessingInstruction() {
        KoXmlPackedItem& item = newItem(currentDepth + 1);
        item.type = KoXmlNode::ProcessingInstructionNode;
    }

public:
    KoXmlPackedDocument(): processNamespace(false), currentDepth(0) {
        clear();
    }

#else

private:
    unsigned elementDepth;

public:

    KoXmlPackedItem& newItem() {
        unsigned count = items.count() + 512;
        count = 1024 * (count >> 10);
        items.reserve(count);

        items.resize(items.count() + 1);

        // this is necessary, because intentionally we don't want to have
        // a constructor for KoXmlPackedItem
        KoXmlPackedItem& item = items[items.count()-1];
        item.attr = false;
        item.type = KoXmlNode::NullNode;
        item.nameIndex = 0;
        item.nsURIIndex = 0;
        item.depth = 0;

        return item;
    }

    void addElement(const QString& name, const QString& nsURI) {
        // we are going one level deeper
        elementDepth++;

        KoXmlPackedItem& item = newItem();

        item.attr = false;
        item.type = KoXmlNode::ElementNode;
        item.depth = elementDepth;
        item.nameIndex = cacheString(name);
        item.nsURIIndex = cacheString(nsURI);
    }

    void closeElement() {
        // we are going up one level
        elementDepth--;
    }

    void addAttribute(const QString& name, const QString& nsURI, const QString& value) {
        KoXmlPackedItem& item = newItem();

        item.attr = true;
        item.type = KoXmlNode::NullNode;
        item.depth = elementDepth;
        item.nameIndex = cacheString(name);
        item.nsURIIndex = cacheString(nsURI);
        //item.value = cacheValue( value );
        item.value = value;
    }

    void addText(const QString& str) {
        KoXmlPackedItem& item = newItem();

        item.attr = false;
        item.type = KoXmlNode::TextNode;
        item.depth = elementDepth + 1;
        item.nameIndex = 0;
        item.nsURIIndex = 0;
        item.value = str;
    }

    void addCData(const QString& str) {
        KoXmlPackedItem& item = newItem();

        item.attr = false;
        item.type = KoXmlNode::CDATASectionNode;
        item.depth = elementDepth + 1;
        item.nameIndex = 0;
        item.nsURIIndex = 0;
        item.value = str;
    }

    void addProcessingInstruction() {
        KoXmlPackedItem& item = newItem();

        item.attr = false;
        item.type = KoXmlNode::ProcessingInstructionNode;
        item.depth = elementDepth + 1;
        item.nameIndex = 0;
        item.nsURIIndex = 0;
        item.value.clear();
    }

    void clear() {
        stringList.clear();
        stringHash.clear();
        valueHash.clear();
        valueList.clear();
        items.clear();
        elementDepth = 0;

        // reserve index #0
        cacheString(".");

        KoXmlPackedItem& rootItem = newItem();
        rootItem.attr = false;
        rootItem.type = KoXmlNode::DocumentNode;
        rootItem.depth = 0;
        rootItem.nsURIIndex = 0;
        rootItem.nameIndex = 0;
    }

    void finish() {
        stringHash.clear();
        valueList.clear();
        valueHash.clear();
        items.squeeze();
    }

    KoXmlPackedDocument(): processNamespace(false), elementDepth(0) {
    }

#endif

};

namespace {

    class ParseError {
    public:
        QString errorMsg;
        int errorLine;
        int errorColumn;
        bool error;

        ParseError() :errorLine(-1), errorColumn(-1), error(false) {}
    };

    void parseElement(QXmlStreamReader &xml, KoXmlPackedDocument &doc);

    // parse one element as if this were a standalone xml document
    ParseError parseDocument(QXmlStreamReader &xml, KoXmlPackedDocument &doc) {
        doc.clear();
        ParseError error;
        xml.readNext();
        while (!xml.atEnd() && xml.tokenType() != QXmlStreamReader::EndDocument) {
            switch (xml.tokenType()) {
            case QXmlStreamReader::StartElement:
                parseElement(xml, doc);
                break;
            case QXmlStreamReader::DTD:
                doc.addDTD(xml.dtdName().toString());
                break;
            case QXmlStreamReader::StartDocument:
                if (!xml.documentEncoding().isEmpty() || !xml.documentVersion().isEmpty()) {
                    doc.addProcessingInstruction();
                }
                break;
            case QXmlStreamReader::ProcessingInstruction:
                doc.addProcessingInstruction();
                break;
            default:
                break;
            }
            xml.readNext();
        }
        if (xml.hasError()) {
            error.error = true;
            error.errorMsg = xml.errorString();
            error.errorColumn = xml.columnNumber();
            error.errorLine = xml.lineNumber();
        } else {
            doc.finish();
        }
        return error;
    }

    void parseElementContents(QXmlStreamReader &xml, KoXmlPackedDocument &doc)
    {
        xml.readNext();
        QString ws;
        bool sawElement = false;
        while (!xml.atEnd()) {
            switch (xml.tokenType()) {
            case QXmlStreamReader::EndElement:
                // if an element contains only whitespace, put it in the dom
                if (!ws.isEmpty() && !sawElement) {
                    doc.addText(ws);
                }
                return;
            case QXmlStreamReader::StartElement:
                sawElement = true;
                parseElement(xml, doc);
                break;
            case QXmlStreamReader::Characters:
                if (xml.isCDATA()) {
                    doc.addCData(xml.text().toString());
                } else if (!xml.isWhitespace()) {
                    doc.addText(xml.text().toString());
                } else if (!sawElement) {
                    ws += xml.text();
                }
                break;
            case QXmlStreamReader::ProcessingInstruction:
                doc.addProcessingInstruction();
                break;
            default:
                break;
            }
            xml.readNext();
        }
    }

    void parseElement(QXmlStreamReader &xml, KoXmlPackedDocument &doc) {
        // reader.tokenType() is now QXmlStreamReader::StartElement
        doc.addElement(xml.qualifiedName().toString(),
                       fixNamespace(xml.namespaceUri().toString()));
        QXmlStreamAttributes attr = xml.attributes();
        QXmlStreamAttributes::const_iterator a = attr.constBegin();
        while (a != attr.constEnd()) {
            doc.addAttribute(a->qualifiedName().toString(),
                             a->namespaceUri().toString(),
                             a->value().toString());
            ++a;
        }
        parseElementContents(xml, doc);
        // reader.tokenType() is now QXmlStreamReader::EndElement
        doc.closeElement();
    }

}


// ==================================================================
//
//         KoXmlNodeData
//
// ==================================================================

class KoXmlNodeData
{
public:

    KoXmlNodeData();
    ~KoXmlNodeData();

    // generic properties
    KoXmlNode::NodeType nodeType;
    QString tagName;
    QString namespaceURI;
    QString prefix;
    QString localName;

#ifdef KOXML_COMPACT
    unsigned nodeDepth;
#endif

    // reference counting
    unsigned long count;
    void ref() {
        count++;
    }
    void unref() {
        if (this == &null) return; if (!--count) delete this;
    }

    // type information
    bool emptyDocument;
    QString nodeName() const;

    // for tree and linked-list
    KoXmlNodeData* parent;
    KoXmlNodeData* prev;
    KoXmlNodeData* next;
    KoXmlNodeData* first;
    KoXmlNodeData* last;

    QString text();

    // node manipulation
    void appendChild(KoXmlNodeData* child);
    void clear();

    // attributes
    inline void setAttribute(const QString& name, const QString& value);
    inline QString attribute(const QString& name, const QString& def) const;
    inline bool hasAttribute(const QString& name) const;
    inline void setAttributeNS(const QString& nsURI, const QString& name, const QString& value);
    inline QString attributeNS(const QString& nsURI, const QString& name, const QString& def) const;
    inline bool hasAttributeNS(const QString& nsURI, const QString& name) const;
    inline void clearAttributes();
    inline QStringList attributeNames() const;

    // for text and CDATA
    QString data() const;
    void setData(const QString& data);

    // reference from within the packed doc
    KoXmlPackedDocument* packedDoc;
    unsigned long nodeIndex;

    // for document node
    bool setContent(QXmlStreamReader *reader,
                    QString* errorMsg = 0, int* errorLine = 0, int* errorColumn = 0);

    // used when doing on-demand (re)parse
    bool loaded;
    void loadChildren(int depth = 1);
    void unloadChildren();

    void dump();

    static KoXmlNodeData null;

    // compatibility
    QDomNode asQDomNode(QDomDocument ownerDoc) const;

private:
    QHash<QString, QString> attr;
    QHash<KoXmlStringPair, QString> attrNS;
    QString textData;
    friend class KoXmlElement;
};

KoXmlNodeData KoXmlNodeData::null;

KoXmlNodeData::KoXmlNodeData() : nodeType(KoXmlNode::NullNode),
#ifdef KOXML_COMPACT
        nodeDepth(0),
#endif
        count(1), emptyDocument(true), parent(0), prev(0), next(0), first(0), last(0),
        packedDoc(0), nodeIndex(0), loaded(false)
{
}

KoXmlNodeData::~KoXmlNodeData()
{
    clear();
}

void KoXmlNodeData::clear()
{
    if (first)
        for (KoXmlNodeData* node = first; node ;) {
            KoXmlNodeData* next = node->next;
            node->unref();
            node = next;
        }

    // only document can delete these
    // normal nodes don't "own" them
    if (nodeType == KoXmlNode::DocumentNode)
        delete packedDoc;

    nodeType = KoXmlNode::NullNode;
    tagName.clear();
    prefix.clear();
    namespaceURI.clear();
    textData.clear();
    packedDoc = 0;

    attr.clear();
    attrNS.clear();

    parent = 0;
    prev = next = 0;
    first = last = 0;

    loaded = false;
}

QString KoXmlNodeData::text()
{
    QString t;

    loadChildren();

    KoXmlNodeData* node = first;
    while (node) {
        switch (node->nodeType) {
        case KoXmlNode::ElementNode:
            t += node->text(); break;
        case KoXmlNode::TextNode:
            t += node->data(); break;
        case KoXmlNode::CDATASectionNode:
            t += node->data(); break;
        default: break;
        }
        node = node->next;
    }

    return t;
}

QString KoXmlNodeData::nodeName() const
{
    switch (nodeType) {
    case KoXmlNode::ElementNode: {
        QString n(tagName);
        if (!prefix.isEmpty())
            n.prepend(':').prepend(prefix);
        return n;
    }
    break;

    case KoXmlNode::TextNode:         return QLatin1String("#text");
    case KoXmlNode::CDATASectionNode: return QLatin1String("#cdata-section");
    case KoXmlNode::DocumentNode:     return QLatin1String("#document");
    case KoXmlNode::DocumentTypeNode: return tagName;

    default: return QString(); break;
    }

    // should not happen
    return QString();
}

void KoXmlNodeData::appendChild(KoXmlNodeData* node)
{
    node->parent = this;
    if (!last)
        first = last = node;
    else {
        last->next = node;
        node->prev = last;
        node->next = 0;
        last = node;
    }
}

void KoXmlNodeData::setAttribute(const QString& name, const QString& value)
{
    attr[ name ] = value;
}

QString KoXmlNodeData::attribute(const QString& name, const QString& def) const
{
    if (attr.contains(name))
        return attr[ name ];
    else
        return def;
}

bool KoXmlNodeData::hasAttribute(const QString& name) const
{
    return attr.contains(name);
}

void KoXmlNodeData::setAttributeNS(const QString& nsURI,
                                   const QString& name, const QString& value)
{
    QString prefix;
    QString localName = name;
    int i = name.indexOf(':');
    if (i != -1) {
        localName = name.mid(i + 1);
        prefix = name.left(i);
    }

    if (prefix.isNull()) return;

    KoXmlStringPair key(nsURI, localName);
    attrNS[ key ] = value;
}

QString KoXmlNodeData::attributeNS(const QString& nsURI, const QString& name,
                                   const QString& def) const
{
    KoXmlStringPair key(nsURI, name);
    return attrNS.value(key, def);
}

bool KoXmlNodeData::hasAttributeNS(const QString& nsURI, const QString& name) const
{
    KoXmlStringPair key(nsURI, name);
    return attrNS.contains(key);
}

void KoXmlNodeData::clearAttributes()
{
    attr.clear();
    attrNS.clear();
}

// FIXME how about namespaced attributes ?
QStringList KoXmlNodeData::attributeNames() const
{
    QStringList result;
    result = attr.keys();

    return result;
}

QString KoXmlNodeData::data() const
{
    return textData;
}

void KoXmlNodeData::setData(const QString& d)
{
    textData = d;
}

bool KoXmlNodeData::setContent(QXmlStreamReader* reader, QString* errorMsg, int* errorLine, int* errorColumn)
{
    if (nodeType != KoXmlNode::DocumentNode)
        return false;

    clear();
    nodeType = KoXmlNode::DocumentNode;

    // sanity checks
    if (!reader) return false;

    delete packedDoc;
    packedDoc = new KoXmlPackedDocument;
    packedDoc->processNamespace = reader->namespaceProcessing();

    ParseError error = parseDocument(*reader, *packedDoc);
    if (error.error) {
        // parsing error has occurred
        if (errorMsg) *errorMsg = error.errorMsg;
        if (errorLine) *errorLine = error.errorLine;
        if (errorColumn)  *errorColumn = error.errorColumn;
        return false;
    }

    // initially load
    loadChildren();

    return true;
}

#ifdef KOXML_COMPACT

void KoXmlNodeData::loadChildren(int depth)
{
    // sanity check
    if (!packedDoc) return;

    // already loaded ?
    if (loaded && (depth <= 1)) return;

    // in case depth is different
    unloadChildren();


    KoXmlNodeData* lastDat = 0;

    unsigned childStop = 0;
    if (nodeIndex == packedDoc->itemCount(nodeDepth) - 1)
        childStop = packedDoc->itemCount(nodeDepth + 1);
    else {
        const KoXmlPackedItem& next = packedDoc->itemAt(nodeDepth, nodeIndex + 1);
        childStop = next.childStart;
    }

    const KoXmlPackedItem& self = packedDoc->itemAt(nodeDepth, nodeIndex);

    for (unsigned i = self.childStart; i < childStop; i++) {
        const KoXmlPackedItem& item = packedDoc->itemAt(nodeDepth + 1, i);
        bool textItem = (item.type == KoXmlNode::TextNode);
        textItem |= (item.type == KoXmlNode::CDATASectionNode);

        // attribute belongs to this node
        if (item.attr) {
            KoQName qname = packedDoc->qnameList[item.qnameIndex];
            QString value = item.value;

            QString prefix;

            QString qName; // with prefix
            QString localName;  // without prefix, i.e. local name

            localName = qName = qname.name;
            int i = qName.indexOf(':');
            if (i != -1) prefix = qName.left(i);
            if (i != -1) localName = qName.mid(i + 1);

            if (packedDoc->processNamespace) {
                setAttributeNS(qname.nsURI, qName, value);
                setAttribute(localName, value);
            } else
                setAttribute(qName, value);
        } else {
            KoQName qname = packedDoc->qnameList[item.qnameIndex];
            QString value = item.value;

            QString nodeName = qname.name;
            QString localName;
            QString prefix;

            if (packedDoc->processNamespace) {
                localName = qname.name;
                int di = qname.name.indexOf(':');
                if (di != -1) {
                    localName = qname.name.mid(di + 1);
                    prefix = qname.name.left(di);
                }
                nodeName = localName;
            }

            // make a node out of this item
            KoXmlNodeData* dat = new KoXmlNodeData;
            dat->nodeIndex = i;
            dat->packedDoc = packedDoc;
            dat->nodeDepth = nodeDepth + 1;
            dat->nodeType = item.type;
            dat->tagName = nodeName;
            dat->localName = localName;
            dat->prefix = prefix;
            dat->namespaceURI = qname.nsURI;
            dat->count = 1;
            dat->parent = this;
            dat->prev = lastDat;
            dat->next = 0;
            dat->first = 0;
            dat->last = 0;
            dat->loaded = false;
            dat->textData = (textItem) ? value : QString();

            // adjust our linked-list
            first = (first) ? first : dat;
            last = dat;
            if (lastDat)
                lastDat->next = dat;
            lastDat = dat;

            // recursive
            if (depth > 1)
                dat->loadChildren(depth - 1);
        }
    }

    loaded = true;
}

#else

void KoXmlNodeData::loadChildren(int depth)
{
    // sanity check
    if (!packedDoc) return;

    // already loaded ?
    if (loaded && (depth <= 1)) return;

    // cause we don't know how deep this node's children already loaded are
    unloadChildren();

    KoXmlNodeData* lastDat = 0;
    int nodeDepth = packedDoc->items[nodeIndex].depth;

    for (int i = nodeIndex + 1; i < packedDoc->items.count(); i++) {
        KoXmlPackedItem& item = packedDoc->items[i];
        bool textItem = (item.type == KoXmlNode::TextNode);
        textItem |= (item.type == KoXmlNode::CDATASectionNode);

        // element already outside our depth
        if (!item.attr && (item.type == KoXmlNode::ElementNode))
            if (item.depth <= (unsigned)nodeDepth)
                break;

        // attribute belongs to this node
        if (item.attr && (item.depth == (unsigned)nodeDepth)) {
            QString name = packedDoc->stringList[item.nameIndex];
            QString nsURI = fixNamespace(packedDoc->stringList[item.nsURIIndex]);
            QString value = item.value;

            QString prefix;

            QString qName; // with prefix
            QString localName;  // without prefix, i.e. local name

            localName = qName = name;
            int i = qName.indexOf(':');
            if (i != -1) prefix = qName.left(i);
            if (i != -1) localName = qName.mid(i + 1);

            if (packedDoc->processNamespace) {
                setAttributeNS(nsURI, qName, value);
                setAttribute(localName, value);
            } else
                setAttribute(name, value);
        }

        // the child node
        if (!item.attr) {
            bool instruction = (item.type == KoXmlNode::ProcessingInstructionNode);
            bool ok = (textItem || instruction)  ? (item.depth == (unsigned)nodeDepth) : (item.depth == (unsigned)nodeDepth + 1);

            ok = (item.depth == (unsigned)nodeDepth + 1);

            if (ok) {
                QString name = packedDoc->stringList[item.nameIndex];
                QString nsURI = fixNamespace(packedDoc->stringList[item.nsURIIndex]);
                QString value = item.value;

                QString nodeName = name;
                QString localName;
                QString prefix;

                if (packedDoc->processNamespace) {
                    localName = name;
                    int di = name.indexOf(':');
                    if (di != -1) {
                        localName = name.mid(di + 1);
                        prefix = name.left(di);
                    }
                    nodeName = localName;
                }

                // make a node out of this item
                KoXmlNodeData* dat = new KoXmlNodeData;
                dat->nodeIndex = i;
                dat->packedDoc = packedDoc;
                dat->nodeType = item.type;
                dat->tagName = nodeName;
                dat->localName = localName;
                dat->prefix = prefix;
                dat->namespaceURI = nsURI;
                dat->count = 1;
                dat->parent = this;
                dat->prev = lastDat;
                dat->next = 0;
                dat->first = 0;
                dat->last = 0;
                dat->loaded = false;
                dat->textData = (textItem) ? value : QString();

                // adjust our linked-list
                first = (first) ? first : dat;
                last = dat;
                if (lastDat)
                    lastDat->next = dat;
                lastDat = dat;

                // recursive
                if (depth > 1)
                    dat->loadChildren(depth - 1);
            }
        }
    }

    loaded = true;
}
#endif

void KoXmlNodeData::unloadChildren()
{
    // sanity check
    if (!packedDoc) return;

    if (!loaded) return;

    if (first)
        for (KoXmlNodeData* node = first; node ;) {
            KoXmlNodeData* next = node->next;
            node->unloadChildren();
            node->unref();
            node = next;
        }

    clearAttributes();
    loaded = false;
    first = last = 0;
}

#ifdef KOXML_COMPACT


static QDomNode itemAsQDomNode(QDomDocument ownerDoc, KoXmlPackedDocument* packedDoc,
                               unsigned nodeDepth, unsigned nodeIndex)
{
    // sanity check
    if (!packedDoc)
        return QDomNode();

    const KoXmlPackedItem& self = packedDoc->itemAt(nodeDepth, nodeIndex);

    unsigned childStop = 0;
    if (nodeIndex == packedDoc->itemCount(nodeDepth) - 1)
        childStop = packedDoc->itemCount(nodeDepth + 1);
    else {
        const KoXmlPackedItem& next = packedDoc->itemAt(nodeDepth, nodeIndex + 1);
        childStop = next.childStart;
    }

    // nothing to do here
    if (self.type == KoXmlNode::NullNode)
        return QDomNode();

    // create the element properly
    if (self.type == KoXmlNode::ElementNode) {
        QDomElement element;

        KoQName qname = packedDoc->qnameList[self.qnameIndex];
        qname.nsURI = fixNamespace(qname.nsURI);

        if (packedDoc->processNamespace)
            element = ownerDoc.createElementNS(qname.nsURI, qname.name);
        else
            element = ownerDoc.createElement(qname.name);

        // check all subnodes for attributes
        for (unsigned i = self.childStart; i < childStop; i++) {
            const KoXmlPackedItem& item = packedDoc->itemAt(nodeDepth + 1, i);
            bool textItem = (item.type == KoXmlNode::TextNode);
            textItem |= (item.type == KoXmlNode::CDATASectionNode);

            // attribute belongs to this node
            if (item.attr) {
                KoQName qname = packedDoc->qnameList[item.qnameIndex];
                qname.nsURI = fixNamespace(qname.nsURI );
                QString value = item.value;

                QString prefix;

                QString qName; // with prefix
                QString localName;  // without prefix, i.e. local name

                localName = qName = qname.name;
                int i = qName.indexOf(':');
                if (i != -1) prefix = qName.left(i);
                if (i != -1) localName = qName.mid(i + 1);

                if (packedDoc->processNamespace) {
                    element.setAttributeNS(qname.nsURI, qName, value);
                    element.setAttribute(localName, value);
                } else
                    element.setAttribute(qname.name, value);
            } else {
                // add it recursively
                QDomNode childNode = itemAsQDomNode(ownerDoc, packedDoc, nodeDepth + 1, i);
                element.appendChild(childNode);
            }
        }

        return element;
    }

    // create the text node
    if (self.type == KoXmlNode::TextNode) {
        QString text = self.value;

        // FIXME: choose CDATA when the value contains special characters
        QDomText textNode = ownerDoc.createTextNode(text);
        return textNode;
    }

    // nothing matches? strange...
    return QDomNode();
}

QDomNode KoXmlNodeData::asQDomNode(QDomDocument ownerDoc) const
{
    return itemAsQDomNode(ownerDoc, packedDoc, nodeDepth, nodeIndex);
}

#else

static QDomNode itemAsQDomNode(QDomDocument ownerDoc, KoXmlPackedDocument* packedDoc,
                               unsigned nodeIndex)
{
    // sanity check
    if (!packedDoc)
        return QDomNode();

    KoXmlPackedItem& item = packedDoc->items[nodeIndex];

    // nothing to do here
    if (item.type == KoXmlNode::NullNode)
        return QDomNode();

    // create the element properly
    if (item.type == KoXmlNode::ElementNode) {
        QDomElement element;

        QString name = packedDoc->stringList[item.nameIndex];
        QString nsURI = fixNamespace(packedDoc->stringList[item.nsURIIndex]);

        if (packedDoc->processNamespace)
            element = ownerDoc.createElementNS(nsURI, name);
        else
            element = ownerDoc.createElement(name);

        // check all subnodes for attributes
        int nodeDepth = item.depth;
        for (int i = nodeIndex + 1; i < packedDoc->items.count(); i++) {
            KoXmlPackedItem& item = packedDoc->items[i];
            bool textItem = (item.type == KoXmlNode::TextNode);
            textItem |= (item.type == KoXmlNode::CDATASectionNode);

            // element already outside our depth
            if (!item.attr && (item.type == KoXmlNode::ElementNode))
                if (item.depth <= (unsigned)nodeDepth)
                    break;

            // attribute belongs to this node
            if (item.attr && (item.depth == (unsigned)nodeDepth)) {
                QString name = packedDoc->stringList[item.nameIndex];
                QString nsURI = fixNamespace(packedDoc->stringList[item.nsURIIndex]);
                QString value = item.value;
                QString prefix;

                QString qName; // with prefix
                QString localName;  // without prefix, i.e. local name

                localName = qName = name;
                int i = qName.indexOf(':');
                if (i != -1) prefix = qName.left(i);
                if (i != -1) localName = qName.mid(i + 1);

                if (packedDoc->processNamespace) {
                    element.setAttributeNS(nsURI, qName, value);
                    element.setAttribute(localName, value);
                } else
                    element.setAttribute(name, value);
            }

            // direct child of this node
            if (!item.attr && (item.depth == (unsigned)nodeDepth + 1)) {
                // add it recursively
                QDomNode childNode = itemAsQDomNode(ownerDoc, packedDoc, i);
                element.appendChild(childNode);
            }
        }

        return element;
    }

    // create the text node
    if (item.type == KoXmlNode::TextNode) {
        QString text = item.value;
        // FIXME: choose CDATA when the value contains special characters
        QDomText textNode = ownerDoc.createTextNode(text);
        return textNode;
    }

    // nothing matches? strange...
    return QDomNode();
}

QDomNode KoXmlNodeData::asQDomNode(QDomDocument ownerDoc) const
{
    return itemAsQDomNode(ownerDoc, packedDoc, nodeIndex);
}

#endif

void KoXmlNodeData::dump()
{
    printf("NodeData %p\n", (void*)this);

    printf("  nodeIndex: %d\n", (int)nodeIndex);
    printf("  packedDoc: %p\n", (void*)packedDoc);

    printf("  nodeType : %d\n", (int)nodeType);
    printf("  tagName: %s\n", qPrintable(tagName));
    printf("  namespaceURI: %s\n", qPrintable(namespaceURI));
    printf("  prefix: %s\n", qPrintable(prefix));
    printf("  localName: %s\n", qPrintable(localName));

    printf("  parent : %p\n", (void*)parent);
    printf("  prev : %p\n", (void*)prev);
    printf("  next : %p\n", (void*)next);
    printf("  first : %p\n", (void*)first);
    printf("  last : %p\n", (void*)last);

    printf("  count: %ld\n", count);

    if (loaded)
        printf("  loaded: TRUE\n");
    else
        printf("  loaded: FALSE\n");
}

// ==================================================================
//
//         KoXmlNode
//
// ==================================================================

// Creates a null node
KoXmlNode::KoXmlNode()
{
    d = &KoXmlNodeData::null;
}

// Destroys this node
KoXmlNode::~KoXmlNode()
{
    if (d)
        if (d != &KoXmlNodeData::null)
            d->unref();

    d = 0;
}

// Creates a copy of another node
KoXmlNode::KoXmlNode(const KoXmlNode& node)
{
    d = node.d;
    d->ref();
}

// Creates a node for specific implementation
KoXmlNode::KoXmlNode(KoXmlNodeData* data)
{
    d = data;
    data->ref();
}

// Creates a shallow copy of another node
KoXmlNode& KoXmlNode::operator=(const KoXmlNode & node)
{
    d->unref();
    d = node.d;
    d->ref();
    return *this;
}

// Note: two null nodes are always equal
bool KoXmlNode::operator==(const KoXmlNode& node) const
{
    if (isNull() && node.isNull()) return true;
    return(d == node.d);
}

// Note: two null nodes are always equal
bool KoXmlNode::operator!=(const KoXmlNode& node) const
{
    if (isNull() && !node.isNull()) return true;
    if (!isNull() && node.isNull()) return true;
    if (isNull() && node.isNull()) return false;
    return(d != node.d);
}

KoXmlNode::NodeType KoXmlNode::nodeType() const
{
    return d->nodeType;
}

bool KoXmlNode::isNull() const
{
    return d->nodeType == NullNode;
}

bool KoXmlNode::isElement() const
{
    return d->nodeType == ElementNode;
}

bool KoXmlNode::isText() const
{
    return (d->nodeType == TextNode) || isCDATASection();
}

bool KoXmlNode::isCDATASection() const
{
    return d->nodeType == CDATASectionNode;
}

bool KoXmlNode::isDocument() const
{
    return d->nodeType == DocumentNode;
}

bool KoXmlNode::isDocumentType() const
{
    return d->nodeType == DocumentTypeNode;
}

void KoXmlNode::clear()
{
    d->unref();
    d = new KoXmlNodeData;
}

QString KoXmlNode::nodeName() const
{
    return d->nodeName();
}

QString KoXmlNode::prefix() const
{
    return isElement() ? d->prefix : QString();
}

QString KoXmlNode::namespaceURI() const
{
    return isElement() ? d->namespaceURI : QString();
}

QString KoXmlNode::localName() const
{
    return isElement() ? d->localName : QString();
}

KoXmlDocument KoXmlNode::ownerDocument() const
{
    KoXmlNodeData* node = d;
    while (node->parent) node = node->parent;

    return KoXmlDocument(node);
}

KoXmlNode KoXmlNode::parentNode() const
{
    return d->parent ? KoXmlNode(d->parent) : KoXmlNode();
}

bool KoXmlNode::hasChildNodes() const
{
    if (isText())
        return false;

    if (!d->loaded)
        d->loadChildren();

    return d->first != 0 ;
}

int KoXmlNode::childNodesCount() const
{
    if (isText())
        return 0;

    if (!d->loaded)
        d->loadChildren();

    KoXmlNodeData* node = d->first;
    int count = 0;
    while (node) {
        count++;
        node = node->next;
    }

    return count;
}

QStringList KoXmlNode::attributeNames() const
{
    if (!d->loaded)
        d->loadChildren();

    return d->attributeNames();
}

KoXmlNode KoXmlNode::firstChild() const
{
    if (!d->loaded)
        d->loadChildren();
    return d->first ? KoXmlNode(d->first) : KoXmlNode();
}

KoXmlNode KoXmlNode::lastChild() const
{
    if (!d->loaded)
        d->loadChildren();
    return d->last ? KoXmlNode(d->last) : KoXmlNode();
}

KoXmlNode KoXmlNode::nextSibling() const
{
    return d->next ? KoXmlNode(d->next) : KoXmlNode();
}

KoXmlNode KoXmlNode::previousSibling() const
{
    return d->prev ? KoXmlNode(d->prev) : KoXmlNode();
}

KoXmlNode KoXmlNode::namedItem(const QString& name) const
{
    if (!d->loaded)
        d->loadChildren();

    KoXmlNodeData* node = d->first;
    while (node) {
        if (node->nodeName() == name)
            return KoXmlNode(node);
        node = node->next;
    }

    // not found
    return KoXmlNode();
}

KoXmlNode KoXmlNode::namedItemNS(const QString& nsURI, const QString& name) const
{
    if (!d->loaded)
        d->loadChildren();

    KoXmlNodeData* node = d->first;
    while (node) {
        if (node->nodeType == KoXmlNode::ElementNode
                 && node->namespaceURI == nsURI
                 && node->localName == name) {
            return KoXmlNode(node);
        }
        node = node->next;
    }

    // not found
    return KoXmlNode();
}

KoXmlElement KoXmlNode::toElement() const
{
    return isElement() ? KoXmlElement(d) : KoXmlElement();
}

KoXmlText KoXmlNode::toText() const
{
    return isText() ? KoXmlText(d) : KoXmlText();
}

KoXmlCDATASection KoXmlNode::toCDATASection() const
{
    return isCDATASection() ? KoXmlCDATASection(d) : KoXmlCDATASection();
}

KoXmlDocument KoXmlNode::toDocument() const
{
    if (isDocument())
        return KoXmlDocument(d);

    KoXmlDocument newDocument;
    newDocument.d->emptyDocument = false;
    return newDocument;
}

void KoXmlNode::load(int depth)
{
    d->loadChildren(depth);
}

void KoXmlNode::unload()
{
    d->unloadChildren();
}

QDomNode KoXmlNode::asQDomNode(QDomDocument ownerDoc) const
{
    return d->asQDomNode(ownerDoc);
}

// ==================================================================
//
//         KoXmlElement
//
// ==================================================================

// Creates an empty element
KoXmlElement::KoXmlElement(): KoXmlNode(new KoXmlNodeData)
{
    // because referenced also once in KoXmlNode constructor
    d->unref();
}

KoXmlElement::~KoXmlElement()
{
    if (d)
        if (d != &KoXmlNodeData::null)
            d->unref();

    d = 0;
}

// Creates a shallow copy of another element
KoXmlElement::KoXmlElement(const KoXmlElement& element): KoXmlNode(element.d)
{
}

KoXmlElement::KoXmlElement(KoXmlNodeData* data): KoXmlNode(data)
{
}

// Copies another element
KoXmlElement& KoXmlElement::operator=(const KoXmlElement & element)
{
    KoXmlNode::operator=(element);
    return *this;
}

bool KoXmlElement::operator== (const KoXmlElement& element) const
{
    if (isNull() || element.isNull()) return false;
    return (d == element.d);
}

bool KoXmlElement::operator!= (const KoXmlElement& element) const
{
    if (isNull() && element.isNull()) return false;
    if (isNull() || element.isNull()) return true;
    return (d != element.d);
}

QString KoXmlElement::tagName() const
{
    return isElement() ? ((KoXmlNodeData*)d)->tagName : QString();
}

QString KoXmlElement::text() const
{
    return d->text();
}

QString KoXmlElement::attribute(const QString& name) const
{
    if (!isElement())
        return QString();

    if (!d->loaded)
        d->loadChildren();

    return d->attribute(name, QString());
}

QString KoXmlElement::attribute(const QString& name,
                                const QString& defaultValue) const
{
    if (!isElement())
        return defaultValue;

    if (!d->loaded)
        d->loadChildren();

    return d->attribute(name, defaultValue);
}

QString KoXmlElement::attributeNS(const QString& namespaceURI,
                                  const QString& localName, const QString& defaultValue) const
{
    if (!isElement())
        return defaultValue;

    if (!d->loaded)
        d->loadChildren();

    KoXmlStringPair key(namespaceURI, localName);
    return d->attrNS.value(key, defaultValue);

//  return d->attributeNS( namespaceURI, localName, defaultValue );
}

bool KoXmlElement::hasAttribute(const QString& name) const
{
    if (!d->loaded)
        d->loadChildren();

    return isElement() ? d->hasAttribute(name) : false;
}

bool KoXmlElement::hasAttributeNS(const QString& namespaceURI,
                                  const QString& localName) const
{
    if (!d->loaded)
        d->loadChildren();

    return isElement() ? d->hasAttributeNS(namespaceURI, localName) : false;
}

// ==================================================================
//
//         KoXmlText
//
// ==================================================================

KoXmlText::KoXmlText(): KoXmlNode(new KoXmlNodeData)
{
    // because referenced also once in KoXmlNode constructor
    d->unref();
}

KoXmlText::~KoXmlText()
{
    if (d)
        if (d != &KoXmlNodeData::null)
            d->unref();

    d = 0;
}

KoXmlText::KoXmlText(const KoXmlText& text): KoXmlNode(text.d)
{
}

KoXmlText::KoXmlText(KoXmlNodeData* data): KoXmlNode(data)
{
}

bool KoXmlText::isText() const
{
    return true;
}

QString KoXmlText::data() const
{
    return d->data();
}

KoXmlText& KoXmlText::operator=(const KoXmlText & element)
{
    KoXmlNode::operator=(element);
    return *this;
}

// ==================================================================
//
//         KoXmlCDATASection
//
// ==================================================================

KoXmlCDATASection::KoXmlCDATASection(): KoXmlText()
{
    d->nodeType = KoXmlNode::CDATASectionNode;
}

KoXmlCDATASection::KoXmlCDATASection(const KoXmlCDATASection& cdata)
        : KoXmlText(cdata)
{
    *this = cdata;
}

KoXmlCDATASection::~KoXmlCDATASection()
{
    d->unref();
    d = 0;
}

KoXmlCDATASection::KoXmlCDATASection(KoXmlNodeData* cdata):
        KoXmlText(cdata)
{
}

bool KoXmlCDATASection::isCDATASection() const
{
    return true;
}

KoXmlCDATASection& KoXmlCDATASection::operator=(const KoXmlCDATASection & cdata)
{
    KoXmlNode::operator=(cdata);
    return *this;
}

// ==================================================================
//
//         KoXmlDocumentType
//
// ==================================================================

KoXmlDocumentType::KoXmlDocumentType(): KoXmlNode(new KoXmlNodeData)
{
    // because referenced also once in KoXmlNode constructor
    d->unref();
}

KoXmlDocumentType::~KoXmlDocumentType()
{
    d->unref();
    d = 0;
}

KoXmlDocumentType::KoXmlDocumentType(const KoXmlDocumentType& dt):
        KoXmlNode(dt.d)
{
}

QString KoXmlDocumentType::name() const
{
    return nodeName();
}

KoXmlDocumentType::KoXmlDocumentType(KoXmlNodeData* dt): KoXmlNode(dt)
{
}

KoXmlDocumentType& KoXmlDocumentType::operator=(const KoXmlDocumentType & dt)
{
    KoXmlNode::operator=(dt);
    return *this;
}

// ==================================================================
//
//         KoXmlDocument
//
// ==================================================================

KoXmlDocument::KoXmlDocument(): KoXmlNode()
{
    d->emptyDocument = false;
}

KoXmlDocument::~KoXmlDocument()
{
    if (d)
        if (d != &KoXmlNodeData::null)
            d->unref();

    d = 0;
}

KoXmlDocument::KoXmlDocument(KoXmlNodeData* data): KoXmlNode(data)
{
    d->emptyDocument = true;
}

// Creates a copy of another document
KoXmlDocument::KoXmlDocument(const KoXmlDocument& doc): KoXmlNode(doc.d)
{
}

// Creates a shallow copy of another document
KoXmlDocument& KoXmlDocument::operator=(const KoXmlDocument & doc)
{
    KoXmlNode::operator=(doc);
    return *this;
}

// Checks if this document and doc are equals
bool KoXmlDocument::operator==(const KoXmlDocument& doc) const
{
    return(d == doc.d);
}

// Checks if this document and doc are not equals
bool KoXmlDocument::operator!=(const KoXmlDocument& doc) const
{
    return(d != doc.d);
}

KoXmlElement KoXmlDocument::documentElement() const
{
    d->loadChildren();

    for (KoXmlNodeData* node = d->first; node;) {
        if (node->nodeType == KoXmlNode::ElementNode)
            return KoXmlElement(node);
        else node = node->next;
    }

    return KoXmlElement();
}

KoXmlDocumentType KoXmlDocument::doctype() const
{
    return dt;
}

QString KoXmlDocument::nodeName() const
{
    if (d->emptyDocument)
        return QLatin1String("#document");
    else
        return QString();
}

void KoXmlDocument::clear()
{
    KoXmlNode::clear();
    d->emptyDocument = false;
}

namespace {
    /* Use an entity resolver that ignores undefined entities and simply
       returns an empty string for them.
       */
    class DumbEntityResolver : public QXmlStreamEntityResolver {
    public:
        QString resolveUndeclaredEntity ( const QString &) { return ""; }
    };

}

bool KoXmlDocument::setContent(QXmlStreamReader *reader,
                               QString* errorMsg, int* errorLine, int* errorColumn)
{
    if (d->nodeType != KoXmlNode::DocumentNode) {
        d->unref();
        d = new KoXmlNodeData;
        d->nodeType = KoXmlNode::DocumentNode;
    }

    dt = KoXmlDocumentType();
    bool result = d->setContent(reader, errorMsg, errorLine, errorColumn);
    if (result && !isNull()) {
        dt.d->nodeType = KoXmlNode::DocumentTypeNode;
        dt.d->tagName = d->packedDoc->docType;
        dt.d->parent = d;
    }

    return result;
}

// no namespace processing
bool KoXmlDocument::setContent(QIODevice* device, QString* errorMsg,
                               int* errorLine, int* errorColumn)
{
    return setContent(device, false, errorMsg, errorLine, errorColumn);
}

bool KoXmlDocument::setContent(QIODevice* device, bool namespaceProcessing,
                               QString* errorMsg, int* errorLine, int* errorColumn)
{
    if (d->nodeType != KoXmlNode::DocumentNode) {
        d->unref();
        d = new KoXmlNodeData;
        d->nodeType = KoXmlNode::DocumentNode;
    }

    device->open(QIODevice::ReadOnly);
    QXmlStreamReader reader(device);
    reader.setNamespaceProcessing(namespaceProcessing);
    DumbEntityResolver entityResolver;
    reader.setEntityResolver(&entityResolver);

    dt = KoXmlDocumentType();
    bool result = d->setContent(&reader, errorMsg, errorLine, errorColumn);
    {
        dt.d->nodeType = KoXmlNode::DocumentTypeNode;
        dt.d->tagName = d->packedDoc->docType;
        dt.d->parent = d;
    }

    return result;
}

bool KoXmlDocument::setContent(const QByteArray& text, bool namespaceProcessing,
                               QString *errorMsg, int *errorLine, int *errorColumn)
{
    QBuffer buffer;
    buffer.setData(text);
    return setContent(&buffer, namespaceProcessing, errorMsg, errorLine, errorColumn);
}

bool KoXmlDocument::setContent(const QString& text, bool namespaceProcessing,
                               QString *errorMsg, int *errorLine, int *errorColumn)
{
    if (d->nodeType != KoXmlNode::DocumentNode) {
        d->unref();
        d = new KoXmlNodeData;
        d->nodeType = KoXmlNode::DocumentNode;
    }

    QXmlStreamReader reader(text);
    reader.setNamespaceProcessing(namespaceProcessing);
    DumbEntityResolver entityResolver;
    reader.setEntityResolver(&entityResolver);

    dt = KoXmlDocumentType();
    bool result = d->setContent(&reader, errorMsg, errorLine, errorColumn);
    if (result && !isNull()) {
        dt.d->nodeType = KoXmlNode::DocumentTypeNode;
        dt.d->tagName = d->packedDoc->docType;
        dt.d->parent = d;
    }

    return result;
}

bool KoXmlDocument::setContent(const QString& text,
                               QString *errorMsg, int *errorLine, int *errorColumn)
{
    return setContent(text, false, errorMsg, errorLine, errorColumn);
}

#endif

// ==================================================================
//
//         functions in KoXml namespace
//
// ==================================================================

KoXmlElement KoXml::namedItemNS(const KoXmlNode& node, const QString& nsURI,
                                const QString& localName)
{
#ifdef KOXML_USE_QDOM
    // David's solution for namedItemNS, only for QDom stuff
    KoXmlNode n = node.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        if (n.isElement() && n.localName() == localName &&
                n.namespaceURI() == nsURI)
            return n.toElement();
    }
    return KoXmlElement();
#else
    return node.namedItemNS(nsURI, localName).toElement();
#endif
}

void KoXml::load(KoXmlNode& node, int depth)
{
#ifdef KOXML_USE_QDOM
    // do nothing, QDom has no on-demand loading
    Q_UNUSED(node);
    Q_UNUSED(depth);
#else
    node.load(depth);
#endif
}


void KoXml::unload(KoXmlNode& node)
{
#ifdef KOXML_USE_QDOM
    // do nothing, QDom has no on-demand unloading
    Q_UNUSED(node);
#else
    node.unload();
#endif
}

int KoXml::childNodesCount(const KoXmlNode& node)
{
#ifdef KOXML_USE_QDOM
    return node.childNodes().count();
#else
    // compatibility function, because no need to implement
    // a class like QDomNodeList
    return node.childNodesCount();
#endif
}

QStringList KoXml::attributeNames(const KoXmlNode& node)
{
#ifdef KOXML_USE_QDOM
    QStringList result;

    QDomNamedNodeMap attrMap = node.attributes();
    for (int i = 0; i < attrMap.count(); i++)
        result += attrMap.item(i).toAttr().name();

    return result;
#else
    // compatibility function, because no need to implement
    // a class like QDomNamedNodeMap
    return node.attributeNames();
#endif
}

QDomNode KoXml::asQDomNode(QDomDocument ownerDoc, const KoXmlNode& node)
{
#ifdef KOXML_USE_QDOM
    Q_UNUSED(ownerDoc);
    return node;
#else
    return node.asQDomNode(ownerDoc);
#endif
}

QDomElement KoXml::asQDomElement(QDomDocument ownerDoc, const KoXmlElement& element)
{
    return KoXml::asQDomNode(ownerDoc, element).toElement();
}

QDomDocument KoXml::asQDomDocument(QDomDocument ownerDoc, const KoXmlDocument& document)
{
    return KoXml::asQDomNode(ownerDoc, document).toDocument();
}

bool KoXml::setDocument(KoXmlDocument& doc, QIODevice* device,
                        bool namespaceProcessing, QString* errorMsg, int* errorLine,
                        int* errorColumn)
{
    QXmlStreamReader reader(device);
    reader.setNamespaceProcessing(namespaceProcessing);
    bool result = doc.setContent(&reader, errorMsg, errorLine, errorColumn);
    return result;
}
