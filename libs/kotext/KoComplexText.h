/****************************************************************************
** $Id$
**
** Internal header file.
**
** Created :
**
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QCOMPLEXTEXT_H
#define QCOMPLEXTEXT_H

#include <koffice_export.h>

#include <qstring.h>
#include <q3pointarray.h>
#include <qfont.h>
#include <qpainter.h>
#include <q3ptrlist.h>
#include <q3shared.h>
#include <QPainter>

// bidi helper classes. Internal to Qt
struct KOTEXT_EXPORT KoBidiStatus {
    KoBidiStatus() {
	eor = QChar::DirON;
	lastStrong = QChar::DirON;
	last = QChar:: DirON;
    }
    QChar::Direction eor;
    QChar::Direction lastStrong;
    QChar::Direction last;
};

struct KOTEXT_EXPORT KoBidiContext : public Q3Shared {
    // ### ref and deref parent?
    KoBidiContext( uchar level, QChar::Direction embedding, KoBidiContext *parent = 0, bool override = FALSE );
    ~KoBidiContext();

    unsigned char level;
    bool override : 1;
    QChar::Direction dir : 5;

    KoBidiContext *parent;
};

struct KOTEXT_EXPORT KoBidiControl {
    KoBidiControl() { context = 0; }
    KoBidiControl( KoBidiContext *c, KoBidiStatus s)
    { context = c; if( context ) context->ref(); status = s; }
    ~KoBidiControl() { if ( context && context->deref() ) delete context; }
    void setContext( KoBidiContext *c ) { if ( context == c ) return; if ( context && context->deref() ) delete context; context = c; context->ref(); }
    KoBidiContext *context;
    KoBidiStatus status;
};

struct KOTEXT_EXPORT KoTextRun {
    KoTextRun(int _start, int _stop, KoBidiContext *context, QChar::Direction dir);

    int start;
    int stop;
    // explicit + implicit levels here
    uchar level;
};

class KOTEXT_EXPORT KoComplexText {
public:
    enum Shape {
	XIsolated,
	XFinal,
	XInitial,
	XMedial
    };
    static Shape glyphVariant( const QString &str, int pos);
    static Shape glyphVariantLogical( const QString &str, int pos);

    static QString shapedString( const QString &str, int from = 0, int len = -1, QPainter::LayoutDirection dir = QPainter::Auto, const QFontMetrics *fm = 0);
    static QChar shapedCharacter(const QString &str, int pos, const QFontMetrics *fm = 0);

    // positions non spacing marks relative to the base character at position pos.
    //static QPointArray positionMarks( QFontPrivate *f, const QString &str, int pos, QRect *boundingRect = 0 );

    static Q3PtrList<KoTextRun> *bidiReorderLine( KoBidiControl *control, const QString &str, int start, int len,
						QChar::Direction basicDir = QChar::DirON );
    static QString bidiReorderString( const QString &str, QChar::Direction basicDir = QChar::DirON );
};

#endif
