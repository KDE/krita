/*
 *  Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef COMPOSITE_CMYK
#define COMPOSITE_CMYK

void compositeCopyCyan(qint32 stride,
                       quint8 *dst,
                       qint32 dststride,
                       quint8 *src,
                       qint32 srcstride,
                       qint32 rows,
                       qint32 cols,
                       quint8 opacity = OPACITY_OPAQUE)
{
    compositeCopyChannel(PIXEL_CYAN, stride, dst, dststride, src, srcstride, rows, cols, opacity);
}


void compositeCopyMagenta(qint32 stride,
                          quint8 *dst,
                          qint32 dststride,
                          quint8 *src,
                          qint32 srcstride,
                          qint32 rows,
                          qint32 cols,
                          quint8 opacity = OPACITY_OPAQUE)
{
    compositeCopyChannel(PIXEL_MAGENTA, stride, dst, dststride, src, srcstride, rows, cols, opacity);

}


void compositeCopyYellow(qint32 stride,
                         quint8 *dst,
                         qint32 dststride,
                         quint8 *src,
                         qint32 srcstride,
                         qint32 rows,
                         qint32 cols,
                         quint8 opacity = OPACITY_OPAQUE)
{
    compositeCopyChannel(PIXEL_YELLOW, stride, dst, dststride, src, srcstride, rows, cols, opacity);

}


void compositeCopyBlack(qint32 stride,
                        quint8 *dst,
                        qint32 dststride,
                        quint8 *src,
                        qint32 srcstride,
                        qint32 rows,
                        qint32 cols,
                        quint8 opacity = OPACITY_OPAQUE)
{
    compositeCopyChannel(PIXEL_BLACK, stride, dst, dststride, src, srcstride, rows, cols, opacity);
}


#endif
