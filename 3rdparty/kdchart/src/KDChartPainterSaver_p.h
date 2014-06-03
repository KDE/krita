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

#ifndef KDCHARTPAINTERSAVER_P_H
#define KDCHARTPAINTERSAVER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KD Chart API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QPainter>

#include <KDABLibFakes>


namespace KDChart {

/**
   \internal

   @short Resource Allocation Is Initialization for QPainter::save and
   restore

   Usage:

   Instead of
   \code
   painter.save();
   // ...
   painter.restore();
   \endcode

   Use this:

   \code
   const QPainterSaver saver( &painter );
   // ...
   \endcode

   which makes sure that restore() is called when exiting from guard
   clauses, or when exceptions are thrown.
*/
class PainterSaver {
    Q_DISABLE_COPY( PainterSaver )
public:
    explicit PainterSaver( QPainter* p )
        : painter( p ) 
    {
#if defined Q_OS_WIN
        static bool initialized = false;
        static bool isQt4_3_0;
        if( !initialized )
        {
            isQt4_3_0 = ( QString::fromLatin1( qVersion() ) == QString::fromLatin1( "4.3.0" ) );
        }
        initialized = true;
        m_isQt4_3_0 = isQt4_3_0;
#endif
        p->save();
    }

    ~PainterSaver()
    {
#if defined Q_OS_WIN
        // the use of setClipRect is a workaround for a bug in Qt 4.3.0 which could
        // lead to an assert on Windows
        if( m_isQt4_3_0 )
        {
            painter->setClipRect( 0, 0, 2, 2 );
        }
#endif
        painter->restore(); 
    }

private:
#if defined Q_OS_WIN
    bool m_isQt4_3_0;
#endif
    QPainter* const painter;
};

}

#endif /* KDCHARTPAINTERSAVER_P_H */

