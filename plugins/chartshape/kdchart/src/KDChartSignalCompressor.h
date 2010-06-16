/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2005-2007 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#ifndef KDCHARTSIGNALCOMPRESSOR_H
#define KDCHARTSIGNALCOMPRESSOR_H

#include <QObject>
#include <QTimer>

namespace KDChart {

    /** SignalCompressor compresses signals where the same signal
        needs to be emitted by several pieces of the code, but only
        one of the signals should be received at the end.
        Usage:
        * create a object of SignalCompressor, and give it the name
          and object of the signal it is supposed to manage
        * instead of emitting the signal, call emitSignal() on the compressor
        * the signal will only be emitted once, and that is after the
          current call stack ends and returns to the event loop

        With the current implementation, the class changes the
        sematics of signals to be a queued connection. If that is not
        wanted, another compression algorithm needs to be
        implemented.
        Also, at the moment, only nullary signals are supported, as
        parameters could not be compressed.
        A typical use of the class is to compress update
        notifications.
        This class is not part of the published KDChart API.
    */
    class SignalCompressor : public QObject
    {
        Q_OBJECT

    public:
        SignalCompressor( QObject* receiver, const char* signal,
                          QObject* parent = 0 );

    Q_SIGNALS:
        void finallyEmit();

    public Q_SLOTS:
        void emitSignal(); // emit() won't work, because of stupid defines

    private Q_SLOTS:
        void nowGoAlready();

    private:
        QTimer m_timer;
    };

}

#endif
