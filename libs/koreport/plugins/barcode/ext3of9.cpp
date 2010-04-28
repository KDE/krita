/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *     This file contains the code that will render the extended 3of9 barcode.
 * This code will parse a string and build a compliant 3of9 string and then
 * call the 3of9 renderer to do the actual drawing.
 */

#include <QString>
#include <QRect>
#include <QPainter>

#include "barcodes.h"

class _ext3of9map
{
public:
    _ext3of9map(const char c, const QString & s) {
        code = c; conversion = s;
    }
    char code;
    QString conversion;
};
const _ext3of9map ext3of9map[] = {
    _ext3of9map('\0' , "%U"),   // NUL
    _ext3of9map('\001' , "$A"),   // SOH
    _ext3of9map('\002' , "$B"),   // STX
    _ext3of9map('\003' , "$C"),   // ETX
    _ext3of9map('\004' , "$D"),   // EOT
    _ext3of9map('\005' , "$E"),   // ENQ
    _ext3of9map('\006' , "$F"),   // ACK
    _ext3of9map('\007' , "$G"),   // BEL
    _ext3of9map('\010' , "$H"),   // BS
    _ext3of9map('\011' , "$I"),   // HT
    _ext3of9map('\012' , "$J"),   // LF
    _ext3of9map('\013' , "$K"),   // VT
    _ext3of9map('\014' , "$L"),   // FF
    _ext3of9map('\015' , "$M"),   // CR
    _ext3of9map('\016' , "$N"),   // SO
    _ext3of9map('\017' , "$O"),   // SI
    _ext3of9map('\020' , "$P"),   // DLE
    _ext3of9map('\021' , "$Q"),   // DC1
    _ext3of9map('\022' , "$R"),   // DC2
    _ext3of9map('\023' , "$S"),   // DC3
    _ext3of9map('\024' , "$T"),   // DC4
    _ext3of9map('\025' , "$U"),   // NAK
    _ext3of9map('\026' , "$V"),   // SYN
    _ext3of9map('\027' , "$W"),   // ETB
    _ext3of9map('\030' , "$X"),   // CAN
    _ext3of9map('\031' , "$Y"),   // EM
    _ext3of9map('\032' , "$Z"),   // SUB
    _ext3of9map('\033' , "%A"),   // ESC
    _ext3of9map('\034' , "%B"),   // FS
    _ext3of9map('\035' , "%C"),   // GS
    _ext3of9map('\036' , "%D"),   // RS
    _ext3of9map('\037' , "%E"),   // US
    _ext3of9map(' ' , " "),   // SPACE
    _ext3of9map('!' , "/A"),
    _ext3of9map('"' , "/B"),
    _ext3of9map('#' , "/C"),
    _ext3of9map('$' , "/D"),
    _ext3of9map('%' , "/E"),
    _ext3of9map('&' , "/F"),
    _ext3of9map('\'' , "/G"),
    _ext3of9map('(' , "/H"),
    _ext3of9map(')' , "/I"),
    _ext3of9map('*' , "/J"),
    _ext3of9map('+' , "/K"),
    _ext3of9map(',' , "/L"),
    _ext3of9map('-' , "-"),   // /M
    _ext3of9map('.' , "."),   // /N
    _ext3of9map('/' , "/O"),
    _ext3of9map('0' , "0"),   // /P
    _ext3of9map('1' , "1"),   // /Q
    _ext3of9map('2' , "2"),   // /R
    _ext3of9map('3' , "3"),   // /S
    _ext3of9map('4' , "4"),   // /T
    _ext3of9map('5' , "5"),   // /U
    _ext3of9map('6' , "6"),   // /V
    _ext3of9map('7' , "7"),   // /W
    _ext3of9map('8' , "8"),   // /X
    _ext3of9map('9' , "9"),   // /Y
    _ext3of9map(':' , "/Z"),
    _ext3of9map(';' , "%F"),
    _ext3of9map('<' , "%G"),
    _ext3of9map('=' , "%H"),
    _ext3of9map('>' , "%I"),
    _ext3of9map('?' , "%J"),
    _ext3of9map('@' , "%V"),
    _ext3of9map('A' , "A"),
    _ext3of9map('B' , "B"),
    _ext3of9map('C' , "C"),
    _ext3of9map('D' , "D"),
    _ext3of9map('E' , "E"),
    _ext3of9map('F' , "F"),
    _ext3of9map('G' , "G"),
    _ext3of9map('H' , "H"),
    _ext3of9map('I' , "I"),
    _ext3of9map('J' , "J"),
    _ext3of9map('K' , "K"),
    _ext3of9map('L' , "L"),
    _ext3of9map('M' , "M"),
    _ext3of9map('N' , "N"),
    _ext3of9map('O' , "O"),
    _ext3of9map('P' , "P"),
    _ext3of9map('Q' , "Q"),
    _ext3of9map('R' , "R"),
    _ext3of9map('S' , "S"),
    _ext3of9map('T' , "T"),
    _ext3of9map('U' , "U"),
    _ext3of9map('V' , "V"),
    _ext3of9map('W' , "W"),
    _ext3of9map('X' , "X"),
    _ext3of9map('Y' , "Y"),
    _ext3of9map('Z' , "Z"),
    _ext3of9map('[' , "%K"),
    _ext3of9map('\\' , "%L"),
    _ext3of9map(']' , "%M"),
    _ext3of9map('^' , "%N"),
    _ext3of9map('_' , "%O"),
    _ext3of9map('`' , "%W"),
    _ext3of9map('a' , "+A"),
    _ext3of9map('b' , "+B"),
    _ext3of9map('c' , "+C"),
    _ext3of9map('d' , "+D"),
    _ext3of9map('e' , "+E"),
    _ext3of9map('f' , "+F"),
    _ext3of9map('g' , "+G"),
    _ext3of9map('h' , "+H"),
    _ext3of9map('i' , "+I"),
    _ext3of9map('j' , "+J"),
    _ext3of9map('k' , "+K"),
    _ext3of9map('l' , "+L"),
    _ext3of9map('m' , "+M"),
    _ext3of9map('n' , "+N"),
    _ext3of9map('o' , "+O"),
    _ext3of9map('p' , "+P"),
    _ext3of9map('q' , "+Q"),
    _ext3of9map('r' , "+R"),
    _ext3of9map('s' , "+S"),
    _ext3of9map('t' , "+T"),
    _ext3of9map('u' , "+U"),
    _ext3of9map('v' , "+V"),
    _ext3of9map('w' , "+W"),
    _ext3of9map('x' , "+X"),
    _ext3of9map('y' , "+Y"),
    _ext3of9map('z' , "+Z"),
    _ext3of9map('{' , "%P"),
    _ext3of9map('|' , "%Q"),
    _ext3of9map('}' , "%R"),
    _ext3of9map('~' , "%S"),
    _ext3of9map('\177' , "%T"),   // DEL

    _ext3of9map(-1 , 0)
};

QString convertTo3of9(QChar c)
{
    for (int i = 0; !ext3of9map[i].conversion.isEmpty(); i++)
        if (ext3of9map[i].code == c.toAscii())
            return ext3of9map[i].conversion;
    return QString();
}


void renderExtended3of9(OROPage * page, const QRectF & r, const QString & str, int align)
{
    QString new_str;
    QChar c;

    for (int i = 0; i < str.length(); i++) {
        c = str.at(i);
        new_str += convertTo3of9(c);
    }

    render3of9(page, r, new_str, align);
}
