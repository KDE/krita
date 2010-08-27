/****************************************************************************
** Copyright (C) 2001-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include "KDTextDocument.h"
#include <QRect>
#include <QAbstractTextDocumentLayout>
#include <QtDebug>
#include <QTextBlock>

#include <KDABLibFakes>

// This is an internal class that mimicks some of the behavior of a
// QLabel with rich text assigned, this is mostly a workaround around
// QTextDocumentLayout not being a public class.

KDTextDocument::KDTextDocument( QObject * p )
    : QTextDocument( p ),
      mHintValid( false ),
      mSizeHint(),
      mMinimumSizeHint()
{

}

KDTextDocument::KDTextDocument( const QString & text, QObject * p )
    : QTextDocument( text, p ),
      mHintValid( false ),
      mSizeHint(),
      mMinimumSizeHint()
{

}

KDTextDocument::~KDTextDocument() {}


QSize KDTextDocument::sizeHint()
{
    if( !mHintValid )
        (void)minimumSizeHint();
    return mSizeHint;
}

QSize KDTextDocument::minimumSizeHint()
{
    /*
    QTextCursor cursor( this );
    if( ! cursor.atEnd() )
        cursor.movePosition( QTextCursor::NextBlock );
    qDebug() << "KDTextDocument::minimumSizeHint() found:" << cursor.block().text();
    QSizeF s( documentLayout()->blockBoundingRect( cursor.block() ).size() );
    qDebug() << "KDTextDocument::minimumSizeHint() found rect" << documentLayout()->blockBoundingRect( cursor.block());
    return QSize( static_cast<int>(s.width()),
                  static_cast<int>(s.height()) );
    */

    if( mHintValid )
        return mMinimumSizeHint;

    mHintValid = true;
    mSizeHint = sizeForWidth( -1 );
    QSize sz(-1, -1);

    // PENDING(kalle) Cache
    sz.rwidth() = sizeForWidth( 0 ).width();
    sz.rheight() = sizeForWidth( 32000 ).height();
    if( mSizeHint.height() < sz.height())
        sz.rheight() = mSizeHint.height();

    mMinimumSizeHint = sz;
    return sz;
}


QSize KDTextDocument::sizeForWidth(int w)
{
    Q_UNUSED( w );

    setPageSize(QSize(0, 100000));

    return documentLayout()->documentSize().toSize();
}
