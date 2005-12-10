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
#include <kis_progress_subject.h>

#include <qobject.h>

#include <kurl.h>
#include <kio/job.h>


#include "kis_id.h"

class QBuffer;
class KisView;

/**
@author Cyrille Berger
*/
class KisScript : public KisProgressSubject /* public QObject */ , public Kross::Api::ScriptContainer
{
    Q_OBJECT
    public:
        /**
         * Create a script from a file on the disk
         */
        KisScript(KURL url, KisView* view, bool execute = false);
        /**
         * Create a script with no code
         */
        KisScript(const QString& name, QString language, KisView* view);
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
    public:
        /**
         * Implementation of the cancel function of 
         */
        virtual void cancel() { /* TODO: how to cancel a script ? */ }
    public:
        void setProgressTotalSteps(Q_INT32 totalSteps);
        void setProgress(Q_INT32 progress);
        void incProgress();
        void setProgressStage(const QString& stage, Q_INT32 progress);
        void setProgressDone();
    private:
        Q_INT32 m_progressSteps, m_progressTotalSteps, m_lastProgressPerCent;
        KURL m_url;
        QBuffer *m_buffer;
        KisID m_id;
        KisView * m_view;
};

#endif
