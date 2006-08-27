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

#ifndef KRITACOREPROGRESS_H
#define KRITACOREPROGRESS_H

#include <kis_progress_subject.h>
#include <krita_export.h>

class KisView;

namespace Kross { namespace KritaCore {

/**
 * TODO: clarify the situation, while, in the future, multiple scripts could be running at a same time,
 * some of the functions are global to all script and some aren't.
 */
class KRITASCRIPTING_EXPORT KritaCoreProgress : public KisProgressSubject
{
        Q_OBJECT
    public:
        KritaCoreProgress(KisView* view);
        virtual ~KritaCoreProgress();

    public:

        /**
         * This function will set this class as the KisProgressSubject in view
         */
        void activateAsSubject();

        virtual void cancel() {}

    public slots:
        void setProgressTotalSteps(uint totalSteps);
        void setProgress(uint progress);
        void incProgress();
        void setProgressStage(const QString& stage, uint progress);
        void progressDone();

        //inline void setPackagePath(QString path) { m_packagePath = path; };
        //inline QString packagePath() { return m_packagePath; }

    private:
        uint m_progressSteps, m_progressTotalSteps, m_lastProgressPerCent;
        KisView * m_view;
        //QString m_packagePath;
};

}}

#endif
