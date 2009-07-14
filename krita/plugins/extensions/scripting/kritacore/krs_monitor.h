/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2007 Sebastian Sauer <mail@dipe.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KRS_MONITOR_H
#define KRS_MONITOR_H

#include <QObject>
#include "krosskritacore_export.h"
//#include <KoScriptingModule.h>

class KisView2;

namespace Scripting
{

class KROSSKRITACORE_EXPORT Monitor : public QObject
{
    Q_OBJECT
private:
    explicit Monitor();
public:
    virtual ~Monitor();
    static Monitor* instance();
    void started();
    void finished();
Q_SIGNALS:
    void signalExecutionStarted();
    void signalExecutionFinished();
};

}

#endif
