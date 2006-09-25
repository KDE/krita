/*
    Copyright (C) 2000, S.R.Haque <shaheedhaque@hotmail.com>.
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    aS32 with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.

DESCRIPTION

    This is a generic parser for Windows MetaFiles (WMFs). The output is
    a series of callbacks (a.k.a. virtual functions) which the caller can
    override as required.

    This is based on code originally written by Stefan Taferner
    (taferner@kde.org).
*/

#ifndef KWMF_H
#define KWMF_H

#include <koffice_export.h>

#include <QList>

#include <q3valuestack.h>

class QDataStream;
class QPolygon;

class KOWMF_EXPORT KWmf
{
public:

    // Construction.

    KWmf(
        unsigned dpi);
    virtual ~KWmf();

    // Called to parse the given file.

    bool parse(
        const QString &file);
    bool parse(
        QDataStream &stream,
        unsigned size);

    class KOWMF_EXPORT DrawContext
    {
    public:
        DrawContext();
        bool m_winding;
        unsigned m_brushColor;
        unsigned m_brushStyle;
        unsigned m_penColor;
        unsigned m_penStyle;
        unsigned m_penWidth;
    };

    // Should be protected...

    void brushSet(
        unsigned color,
        unsigned style);
    void penSet(
        unsigned color,
        unsigned style,
        unsigned width);

protected:
    // Override to get results of parsing.

    virtual void gotEllipse(
        const DrawContext &dc,
        QString type,
        QPoint topLeft,
        QSize halfAxes,
        unsigned startAngle,
        unsigned stopAngle) = 0;
    virtual void gotPolygon(
        const DrawContext &dc,
        const QPolygon &points) = 0;
    virtual void gotPolyline(
        const DrawContext &dc,
        const QPolygon &points) = 0;
    virtual void gotRectangle(
        const DrawContext &dc,
        const QPolygon &points) = 0;

private:
    // Debug support.

    static const int s_area;

    // Use unambiguous names for Microsoft types.

    typedef short S16;
    typedef int S32;
    typedef unsigned int U32;

    int m_dpi;
    int m_windowOrgX;
    int m_windowOrgY;
    int m_windowFlipX;
    int m_windowFlipY;
    DrawContext m_dc;
    Q3ValueStack<DrawContext> m_savedDcs;
    QPoint m_lineFrom;

    // Windows handle management.

    class WinObjHandle
    {
    public:
        virtual ~WinObjHandle () {}
        virtual void apply(KWmf &p) = 0;
    };

    class WinObjBrushHandle: public WinObjHandle
    {
    public:
        virtual void apply(KWmf &p);
        unsigned m_color;
        unsigned m_style;
    };

    class WinObjPenHandle: public WinObjHandle
    {
    public:
        virtual void apply(KWmf &p);
        unsigned m_color;
        unsigned m_style;
        unsigned m_width;
    };

    WinObjPenHandle *handleCreatePen(void);
    WinObjBrushHandle *handleCreateBrush(void);
    QList<WinObjHandle *>m_objectHandles;

    unsigned getColor(S32 color);
    QPoint normalisePoint(
        QDataStream &operands);
    QSize normaliseSize(
        QDataStream &operands);
    void genericArc(
        QString type,
        QDataStream &operands);

    // Opcode handling and painter methods.

    void walk(
        U32 words,
        QDataStream &stream);
    void skip(
        U32 words,
        QDataStream &operands);
    void invokeHandler(
        S16 opcode,
        U32 words,
        QDataStream &operands);
/*
    // draw multiple polygons
    void opPolypolygon(U32 words, QDataStream &operands);
*/
    void opArc(U32 words, QDataStream &operands);
    // create a logical brush
    void opBrushCreateIndirect(U32 words, QDataStream &operands);
    void opEllipse(U32 words, QDataStream &operands);
    // draw line to coord
    void opLineTo(U32 words, QDataStream &operands);
    // move pen to coord
    void opMoveTo(U32 words, QDataStream &operands);
    // do nothing
    void opNoop(U32 words, QDataStream &operands);
    // Free object handle
    void opObjectDelete(U32 words, QDataStream &operands);
    // Activate object handle
    void opObjectSelect(U32 words, QDataStream &operands);
    // create a logical pen
    void opPenCreateIndirect(U32 words, QDataStream &operands);
    void opPie(U32 words, QDataStream &operands);
    // draw polygon
    void opPolygon(U32 words, QDataStream &operands);
    // set polygon fill mode
    void opPolygonSetFillMode(U32 words, QDataStream &operands);
    // draw series of lines
    void opPolyline(U32 words, QDataStream &operands);
    void opRectangle(U32 words, QDataStream &operands);
    // restore drawing context
    void opRestoreDc(U32 words, QDataStream &operands);
    // save drawing context
    void opSaveDc(U32 words, QDataStream &operands);
    // set window origin
    void opWindowSetOrg(U32 words, QDataStream &operands);
    // set window extents
    void opWindowSetExt(U32 words, QDataStream &operands);
/*
    // set background pen color
    void opsetBkColor(U32 words, QDataStream &operands);
    // set background pen mode
    void opsetBkMode(U32 words, QDataStream &operands);
    // Set raster operation mode
    void opsetRop(U32 words, QDataStream &operands);
    // Escape (enhanced command set)
    void opescape(U32 words, QDataStream &operands);
*/
};

#endif
