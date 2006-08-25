/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#ifndef SCRIPTINGPROGRESS_H
#define SCRIPTINGPROGRESS_H

#include <kis_progress_subject.h>
#include <krita_export.h>

class KisView;

/**
 * TODO: clarify the situation, while, in the future, multiple scripts could be running at a same time,
 * some of the functions are global to all script and some aren't.
 */
class KRITASCRIPTING_EXPORT ScriptingProgress : public KisProgressSubject
{
        Q_OBJECT
    public:
        ScriptingProgress(KisView* view) : KisProgressSubject(), m_view(view) {}
        virtual ~ScriptingProgress() {}

    public:

        /**
         * This function will set this class as the KisProgressSubject in view
         */
        void activateAsSubject();
        virtual void cancel() {}

    public slots:
        void setProgressTotalSteps(qint32 totalSteps);
        void setProgress(qint32 progress);
        void incProgress();
        void setProgressStage(const QString& stage, qint32 progress);
        void progressDone();
        //inline void setPackagePath(QString path) { m_packagePath = path; };
        //inline QString packagePath() { return m_packagePath; }
    private:
        qint32 m_progressSteps, m_progressTotalSteps, m_lastProgressPerCent;
        KisView * m_view;
        //QString m_packagePath;
};

#endif
