/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_SCRIPT_H
#define KIS_SCRIPT_H

#include <main/scriptcontainer.h>
#include <qobject.h>

#include <kurl.h>
#include <kio/job.h>

#include "kis_id.h"

class QBuffer;

/**
@author Cyrille Berger
*/
class KisScript : public QObject, public Kross::Api::ScriptContainer
{
    Q_OBJECT
    public:
        /**
         * Create a script from a file on the disk
         */
        KisScript(KURL url, bool execute = false);
        /**
         * Create a script with no code
         */
        KisScript(const QString& name, QString language);
        ~KisScript();
    public:
        KisID id() { return m_id; }
        /**
         * This function load the scritps from the URL
         */
        void reload();
    public slots:
        /**
         * Call this function to execute a script
         */
        void execute();
    private slots: // Those functions are used to load a file
        void slotData(KIO::Job*, const QByteArray&);
        void slotResult(KIO::Job*);
    signals:
        void loaded();
    private:
        KURL m_url;
        QBuffer *m_buffer;
        KisID m_id;
};

#endif
