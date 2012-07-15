/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2008 Inge Wallin     <inge@lysator.liu.se>

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


// Own
#include "CellRegion.h"

// C
#include <cmath>

// Qt
#include <QPoint>
#include <QRect>
#include <QVector>
#include <QDebug>
#include <QStringList>

// KDE
#include <kdebug.h>

// KChart
#include "TableSource.h"


using std::pow;
using namespace KChart;

/************************RegionParser*******************************/

class Parser
{
public:
    Parser(const QString & input)
        : m_input(input)
        , m_pos(m_input.constEnd())
    {
        m_delimiter.append(QChar::fromLatin1('.'));
        m_delimiter.append(QChar::fromLatin1(':'));
        m_delimiter.append(QChar::fromLatin1(';'));
        m_delimiter.append(QChar::fromLatin1(' '));
    }
    bool parse();
    QList< QRect > getResult() const { return m_result; }
    QString tableName() const { return m_tableName; }

private:
    struct Token
    {
        enum TokenType{ Dot = 0, DoubleDot = 1, Space = 2, Spacer = 3, Identifier = 4, End };
        Token(TokenType type, const QString & identifier): m_type(type), m_identifier(identifier){}
        Token(): m_type(End) {}
        TokenType m_type;
        QString m_identifier;
    };

    inline Token parseToken();
    inline void eatWhitespaces();
    inline bool parseRegionList();
    inline bool parseRegion();
    inline bool parseName();
    inline bool parseName2();
    inline bool parsePoint();
    inline bool parseRegion2();
    inline void setTableName(const QString &name);

private:
    const QString m_input;
    QString::ConstIterator m_pos;
    QList< QRect > m_result;
    Token m_currentToken;
    QRect m_currentRect;
    QPoint m_currentPoint;
    QString m_tableName;
    int m_index;
    QVector< QChar > m_delimiter;
};

void Parser::setTableName(const QString &name)
{
    QString strippedName = name;
    if (name.startsWith(QChar::fromLatin1('$')))
        strippedName.remove(0, 1);
    if (m_tableName.isEmpty())        
        m_tableName = strippedName;
    else
        if (strippedName != m_tableName)
            kDebug() << "More than one sheet referenced, this is currently not supported";
}

bool Parser::parse()
{
    //qDebug() << "Input " << m_input;
    m_pos = m_input.constBegin();
    m_index = 0;
    m_currentToken = parseToken();
    return parseRegionList();
}

Parser::Token Parser::parseToken()
{
    Token::TokenType type = Token::End;
    if (m_pos != m_input.constEnd()) {
        switch(m_delimiter.indexOf(*m_pos)) {
        case(0):
            type = Token::Dot;
            break;
        case(1):
            type = Token::DoubleDot;
            break;
        case(2):
        case(3):
            type = Token::Space;
            break;
        default:
            type = Token::Identifier;
        }
    }
    bool dollarPrefix = false;
    if (m_index >= m_input.size())
        type = Token::End;
    else if (*m_pos == QChar::fromLatin1('$')) {
        ++m_pos;
        ++m_index;
    }
    QString identifier;
    if (m_pos != m_input.constEnd() && *m_pos == QChar::fromLatin1('\'')) {
        ++m_pos;
        ++m_index;
        int startPos = m_index;
        for (; m_pos != m_input.constEnd() && *m_pos != QChar::fromLatin1('\''); ++m_pos, ++m_index)
            ;

        if (type == Token::Identifier)
            identifier = m_input.mid(startPos, m_index - startPos);
        if (m_pos != m_input.constEnd()) {
            ++m_pos;
            ++m_index;
        }
    }
    else {
        int startPos = m_index;
        for (; m_pos != m_input.constEnd() && !m_delimiter.contains(*m_pos); ++m_pos, ++m_index)
            ;
        if (m_pos != m_input.constEnd() && startPos == m_index) {
            ++m_index;
            ++m_pos;
        }
        if (type == Token::Identifier)
            identifier = m_input.mid(startPos, m_index - startPos);
    }

    return Token(type, identifier);
}

void Parser::eatWhitespaces()
{
    for (; m_pos != m_input.constEnd() && *m_pos == QChar::fromLatin1(' '); ++m_pos, ++m_index)
        ;
}

bool Parser::parseRegionList()
{
    bool res = true;
    for (; m_currentToken.m_type != Token::End; m_currentToken = parseToken()) {
        if (m_currentToken.m_type != Token::Space) {
            if (m_currentToken.m_type == Token::Identifier)
                res = parseRegion();
            else
                res = false;
        }
    }
    return res;
}

bool Parser::parseRegion()
{
    //qDebug() << "parseRegion";
    bool res = true;
    res &= parseRegion2();
    m_currentToken = parseToken();
    //qDebug() << "CurrentToken " << m_currentToken.m_identifier << m_currentToken.m_type;
    if (m_currentToken.m_type == Token::DoubleDot) {
        const QPoint topLeft = m_currentPoint;
        m_currentToken = parseToken();
        res &= parseRegion2();
        //m_currentToken = parseToken();
        m_result.append(QRect(topLeft, m_currentPoint));
        //qDebug() << "DoubleDot";
    }
    else {
        m_result.append(QRect(m_currentPoint, m_currentPoint));
        //qDebug() << "NODoubleDot";
    }
    
    if (m_currentToken.m_type == Token::Space)
        res &= parseRegionList();
    else if (m_currentToken.m_type == Token::End)
        return res;
    else
        res = false;

    return res;
}

bool Parser::parseRegion2()
{
    //qDebug() << "ParseRegion2";
    bool res = true;

    if (m_currentToken.m_type != Token::Identifier && m_currentToken.m_type != Token::Dot)
        res = false;

    const QString firstIdentifier = m_currentToken.m_type != Token::Dot ? m_currentToken.m_identifier : tableName();
    if (m_currentToken.m_type != Token::Dot)
        m_currentToken = parseToken();
    if (m_currentToken.m_type == Token::Dot)
    {
        m_currentToken = parseToken();
        if (m_currentToken.m_type == Token::Identifier)
        {            
            QRegExp regEx(QString::fromLatin1("([$]*)([A-Z]+)([$]*)([0-9]+)"));
            regEx.exactMatch(m_currentToken.m_identifier);
            m_currentPoint = QPoint(CellRegion::rangeStringToInt(regEx.cap(2)), regEx.cap(4).toInt());
            //qDebug() << "FUN" << regEx.cap(2) << " " << regEx.cap(4);
            setTableName(firstIdentifier);
        }
        else
            res = false;
    }
    else
    {
        QRegExp regEx(QString::fromLatin1("([$]*)([A-Z]+)([$]*)([0-9]+)"));
        regEx.exactMatch(firstIdentifier);
        //qDebug() << "FUN" << regEx.cap(2) << " " << regEx.cap(4);
        m_currentPoint = QPoint(CellRegion::rangeStringToInt(regEx.cap(2)), regEx.cap(4).toInt());
    }
    //qDebug() << "TableName "<< m_tableName;
    //qDebug() << firstIdentifier;
    //qDebug() << "Point" << m_currentPoint;
    //qDebug() << m_currentToken.m_identifier;
    //qDebug() << m_currentToken.m_type;

    return res;

}

// bool Parser::parsePoint()
// {
//     bool res = true;
//     int startIndex = m_index;
//     while(m_pos != m_input.end() && *m_pos != QChar::fromLatin1(':'))
//     {
//         ++m_pos;
//         ++m_index;
//     }
//     const QString currentString = m_input.mid(startIndex, m_index - startIndex);
//     qDebug() << "PointString" << currentString;
//     QRegExp regEx(QString::fromLatin1("[A-Z]+[0-9]+"));
//     regEx.indexIn(currentString);
//     m_currentPoint = QPoint(CellRegion::rangeStringToInt(regEx.cap(0)), regEx.cap(1).toInt());
//     return res;
// }
/************************ENDRegionParser*******************************/

static QString columnName(uint column);
//static int rangeCharToInt(char c);

/**
 * Makes sure that quotes are added if name contains spaces or special
 * characters. May also be used to escape certain characters if needed.
 */
static QString formatTableName(QString name)
{
    static const QList<QChar> specialChars =
        QList<QChar>() << ' ' << '\t' << '-' << '\'';

    bool containsSpecialChars = false;
    foreach(QChar c, specialChars)
        containsSpecialChars = containsSpecialChars || name.contains(c);

    if(containsSpecialChars)
        name.prepend('\'').append('\'');

    return name;
}

/**
 * Reverts any operation done by formatTableName(), so that ideally
 * unformatTableName(formatTableName(name)) == name
 */
static QString unformatTableName(QString name)
{
    if (name.startsWith('\'') && name.endsWith('\'')) {
        name.remove(0, 1);
        name.remove(name.length() - 1, 1);
    }

    return name;
}

class CellRegion::Private
{
public:
    Private();
    ~Private();

    QString pointToString(const QPoint &point) const;

    // These are actually one-dimensional, but can have different
    // orientations (hor / vert).
    QVector<QRect> rects;

    QRect          boundingRect;
    // NOTE: Don't forget to extend operator=() if you add new members

    /// Table this region is in (name/model pair provided by TableSource)
    Table *table;
};


CellRegion::Private::Private()
{
    table = 0;
}

CellRegion::Private::~Private()
{
}


// ================================================================
//                         Class CellRegion


CellRegion::CellRegion()
    : d(new Private())
{
}

CellRegion::CellRegion(const CellRegion &region)
    : d(new Private())
{
    // Use operator=();
    *this = region;
}

CellRegion::CellRegion(TableSource *source, const QString& regions)
    : d(new Private())
{
    // A dollar sign before a part of the address means that this part
    // is absolute. This is irrelevant for us, however, thus we can remove
    // all occurrences of '$', and handle relative and absolute addresses in
    // the same way.
    // See ODF specs $8.3.1 "Referencing Table Cells"
    Parser parser(regions);
    const bool success = parser.parse();
    if (!success)
        kDebug() << "Parsing cell region failed";
    d->rects = parser.getResult().toVector();
    d->table = source->get(parser.tableName());
//     QStringList regionsList = regions.split(" ", QString::SkipEmptyParts);
//     Q_FOREACH(const QString& region, regionsList) {
//       QString searchStr = QString(region).remove("$");
//       QRegExp regEx;
// 
//       QStringList regionList = searchStr.split(";");
//       Q_FOREACH(const QString &region, regionList) {
//           const bool isPoint = !region.contains(':');
//           if (isPoint)
//               regEx = QRegExp("(|.*\\.)([A-Z]+)([0-9]+)");
//           else // support range-notations like Sheet1.D2:Sheet1.F2 Sheet1.D2:F2 D2:F2
//               regEx = QRegExp ("(|.*\\.)([A-Z]+)([0-9]+)\\:(|.*\\.)([A-Z]+)([0-9]+)");
// 
//           // Check if region string is valid (e.g. not empty)
//           if (regEx.indexIn(region) >= 0) {
//               // It is possible for a cell-range-address as defined in ODF to contain
//               // refernces to cells of more than one sheet. This, however, we ignore
//               // here. We do not support more than one table in a cell region.
//               // Also we do not support regions spanned over different sheets. For us
//               // everything is either on no sheet or on the same sheet.
//               QString sheetName = regEx.cap(1);
//               if (sheetName.endsWith("."))
//                   sheetName = sheetName.left(sheetName.length() - 1);
//               // TODO: Support for multiple tables in one region
//               d->table = source->get(unformatTableName(sheetName));
// 
//               QPoint topLeft(rangeStringToInt(regEx.cap(2)), regEx.cap(3).toInt());
//               if (isPoint) {
//                   d->rects.append(QRect(topLeft, QSize(1, 1)));
//               } else {
//                   QPoint bottomRight(rangeStringToInt(regEx.cap(5)), regEx.cap(6).toInt());
//                   d->rects.append(QRect(topLeft, bottomRight));
//               }
//           }
//       }
//     }
}

CellRegion::CellRegion(Table *table, const QPoint &point)
    : d(new Private())
{
    d->table = table;
    add(point);
}

CellRegion::CellRegion(Table *table, const QRect &rect)
    : d(new Private())
{
    d->table = table;
    add(rect);
}

CellRegion::CellRegion(Table *table, const QVector<QRect> &rects)
    : d(new Private())
{
    d->table = table;
    foreach(const QRect& rect, rects)
        add(rect);
}

CellRegion::CellRegion(Table *table)
    : d(new Private())
{
    d->table = table;
}

CellRegion::~CellRegion()
{
    delete d;
}


CellRegion& CellRegion::operator = (const CellRegion& region)
{
    d->rects        = region.d->rects;
    d->boundingRect = region.d->boundingRect;
    d->table        = region.d->table;

    return *this;
}

bool CellRegion::operator == (const CellRegion &other) const
{
    return d->rects == other.d->rects;
}


Table *CellRegion::table() const
{
    return d->table;
}

QVector<QRect> CellRegion::rects() const
{
    return d->rects;
}

int CellRegion::rectCount() const
{
    return d->rects.size();
}

QString CellRegion::sheetName() const
{
    return d->table->name();
}

bool CellRegion::isValid() const
{
    return d->rects.size() > 0 && d->table ;
}

QString CellRegion::Private::pointToString(const QPoint &point) const
{
    QString result;

    result.append('$' + columnName(point.x()));
    result.append('$' + QString::number(point.y()));

    return result;
}

QString CellRegion::toString() const
{
    if (!isValid())
        return QString();

    QString result;
    for (int i = 0; i < d->rects.count(); ++i) {
        const QRect range = d->rects[i];
        // Top-left corner
        if (table())
            result.append('$' + formatTableName(table()->name()) + '.');
        result.append(d->pointToString(range.topLeft()));

        // If it is not a point, append rect's bottom-right corner
        if (range.topLeft() != range.bottomRight()) {
            result.append(':');
            result.append(d->pointToString(range.bottomRight()));
        }

        // Separate ranges by a comma, except for the last one
        if (i < d->rects.count() - 1)
            result.append(';');
    }
    return result;
}


bool CellRegion::contains(const QPoint &point, bool proper) const
{
    foreach (const QRect &rect, d->rects) {
        if (rect.contains(point, proper))
            return true;
    }

    return false;
}

bool CellRegion::contains(const QRect &rect, bool proper) const
{
    foreach (const QRect &r, d->rects) {
        if (r.contains(rect, proper))
            return true;
    }

    return false;
}

bool CellRegion::intersects(const CellRegion &other) const
{
    // If both regions lie within only one table and these tables
    // are different, they trivially do not intersect.
    if (table() && other.table() &&
         table() != other.table())
        return false;

    foreach (const QRect &r, d->rects) {
        foreach(const QRect &_r, other.d->rects) {
            if (r.intersects(_r))
                return true;
        }
    }

    return false;
}

CellRegion CellRegion::intersected(const QRect &rect) const
{
    CellRegion intersections;

    foreach (const QRect &r, d->rects) {
        if (r.intersects(rect))
            intersections.add(r.intersected(rect));
    }

    return intersections;
}

Qt::Orientation CellRegion::orientation() const
{
    foreach (const QRect &rect, d->rects) {
        if (rect.width() > 1)
                return Qt::Horizontal;
        if (rect.height() > 1)
                return Qt::Vertical;
    }

    // Default if region is only one cell
    return Qt::Vertical;
}

int CellRegion::cellCount() const
{
    int count = 0;

    /*FIXME the following would be more correct cause it
     * would also cover multi-dimensional ranges (means
     * where rect.width()>1 *and* rect.height()>1). But
     * for that kchart needs lot of fixing (e.g. in
     * the CellRegion to proper handle multi-dimension
     * ranges too).
     *
    foreach (const QRect &rect, d->rects)
        count += (rect.width() * rect.height());
    */

    if (orientation() == Qt::Horizontal) {
        foreach (const QRect &rect, d->rects)
            count += rect.width();
    }
    else {
        foreach(const QRect &rect, d->rects)
            count += rect.height();
    }
    return count;
}

void CellRegion::add(const CellRegion &other)
{
    add(other.rects());
}

void CellRegion::add(const QPoint &point)
{
    add(QRect(point, QSize(1, 1)));
}

void CellRegion::add(const QRect &rect)
{
// These checks are obsolete, a CellRegion can be used otherwise as well
#if 0
    if (!rect.isValid()) {
        qWarning() << "CellRegion::add() Attempt to add invalid rectangle";
        qWarning() << "CellRegion::add():" << rect;
        return;
    }

    if (rect.width() > 1 && rect.height() > 1) {
        qWarning() << "CellRegion::add() Attempt to add rectangle with height AND width > 1";
        qWarning() << "CellRegion::add():" << rect;
        return;
    }
#endif

    d->rects.append(rect);
    d->boundingRect |= rect;
}

void CellRegion::add(const QVector<QRect> &rects)
{
    foreach (const QRect &rect, rects)
        add(rect);
}

QRect CellRegion::boundingRect() const
{
    return d->boundingRect;
}

bool CellRegion::hasPointAtIndex(int index) const
{
    return pointAtIndex(index) != QPoint(-1, -1);
}

QPoint CellRegion::pointAtIndex(int index) const
{
    // sum of all previous rectangle indices
    int i = 0;

    foreach (const QRect &rect, d->rects) {
        // Rectangle is horizontal
        if (rect.width() > 1) {
            // Found it!
            // Index refers to point in current rectangle
            if (i + rect.width() > index) {
                // Local index of point in this rectangle
                int j = index - i;
                return QPoint(rect.x() + j, rect.y());
            }

            // add number of indices in current rectangle to total index count
            i += rect.width();
        }
        else {
            // Found it!
            // Index refers to point in current rectangle
            if (i + rect.height() > index) {
                // Local index of point in this rectangle
                int j = index - i;
                return QPoint(rect.x(), rect.y() + j);
            }

            // add number of indices in current rectangle to total index count
            i += rect.height();
        }
    }

    // Invalid index!
    return QPoint(-1, -1);
}

int CellRegion::indexAtPoint(const QPoint &point) const
{
    int indicesLeftToPoint = 0;
    bool found = false;

    foreach (const QRect &rect, d->rects) {
        if (!rect.contains(point)) {
            indicesLeftToPoint += rect.width() > 1 ? rect.width() : rect.height();
            continue;
        }

        found = true;
        if (rect.width() > 1)
            indicesLeftToPoint += point.x() - rect.topLeft().x();
        else
            indicesLeftToPoint += point.y() - rect.topLeft().y();
    }

    return found ? indicesLeftToPoint : -1;
}

#if 0 // Unused?
static int rangeCharToInt(char c)
{
    return (c >= 'A' && c <= 'Z') ? (c - 'A' + 1) : -1;
}

static int rangeStringToInt(const QString &string)
{
    int result = 0;
    const int size = string.size();
    for (int i = 0; i < size; i++) {
        //kDebug(350001) << "---" << float(rangeCharToInt(string[i].toAscii()) * pow(10.0, (size - i - 1)));
        result += rangeCharToInt(string[i].toAscii()) * pow(10.0, (size - i - 1));
    }
    //kDebug(350001) << "+++++ result=" << result;
    return result;
}

static QString rangeIntToString(int i)
{
    QString tmp = QString::number(i);
    for (int j = 0; j < tmp.size(); j++) {
        tmp[j] = 'A' + tmp[j].toAscii() - '1';
    }

    //kDebug(350001) << "tmp=" << tmp;
    return tmp;
}
#endif

int CellRegion::rangeCharToInt(char c)
{
    return (c >= 'A' && c <= 'Z') ? (c - 'A' + 1) : -1;
}

int CellRegion::rangeStringToInt(const QString &string)
{
    int result = 0;
    const int size = string.size();
    for (int i = 0; i < size; i++) {
        result += rangeCharToInt(string[i].toAscii()) * pow(10.0, (size - i - 1));
    }

    return result;
}

QString CellRegion::rangeIntToString(int i)
{
    QString tmp = QString::number(i);
    for (int j = 0; j < tmp.size(); j++) {
        tmp[j] = 'A' + tmp[j].toAscii() - '1';
    }

    return tmp;
}

// Return the symbolic name of any column.
static QString columnName(uint column)
{
    if (column < 1 || column > 32767)
        return QString("@@@");

    QString   str;
    unsigned  digits = 1;
    unsigned  offset = 0;

    column--;

    for (unsigned limit = 26; column >= limit + offset; limit *= 26, ++digits)
        offset += limit;

    for (unsigned col = column - offset; digits; --digits, col /= 26)
        str.prepend(QChar('A' + (col % 26)));

    return str;
}
