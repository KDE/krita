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

#ifndef KRS_PROGRESS_H
#define KRS_PROGRESS_H

#include "krosskritacore_export.h"

#include <QObject>

class KisView2;
class KoMainWindow;

namespace Scripting
{

class Module;

/**
 * The Progress object enables displaying of a progressbar
 * in Krita to visualize the progress your script makes.
 *
 * Example (in Python) :
 * @code
 * import Krita
 * progress = Krita.progress()
 * progress.setProgressTotalSteps(100)
 * for i in range(100):
 *     progress.incProgress()
 * @endcode
 *
 * Example (in Ruby) :
 * @code
 * require 'Krita'
 * progress = Krita.progress()
 * progress.setProgressTotalSteps(100)
 * for i in 0..100
 *   progress.incProgress()
 * end
 * @endcode
 */
class KROSSKRITACORE_EXPORT Progress : public QObject
{
    Q_OBJECT
public:
    Progress(Module* module, KisView2* view);
    virtual ~Progress();

public:
    void activateAsSubject();
    void updateProgress(int progressPerCent);

public slots:

    /**
     * Set the total steps the progressbar should have. This
     * could be as example the width * height of an image
     * and if you iterate over the images, just call
     * \a incProgress() to increase the current stage.
     * If this function got called, Krita starts to display
     * the progressbar.
     */
    void setProgressTotalSteps(uint totalSteps);

    /**
     * Increment the progress by one step.
     */
    void incProgress();

    /**
     * Set the current stage to \p progress . The value
     * should be always <= as the with \a setProgressTotalSteps()
     * defined maximal number of steps.
     */
    void setProgress(uint progress);

    /**
     * Set the current stage to \p progress and provide with
     * \p stage a in the progressbar displayed string.
     */
    void setProgressStage(const QString& stage, uint progress);

    /**
     * If called, the progressbar will be disabled again do
     * indicate, that the operation is done. Please note, that
     * it's not needed to call this explicit since once the
     * script finished, it's called automatically.
     */
    void progressDone();

private:
    Module* m_module;
    KisView2* m_view;
    KoMainWindow* m_mainwin;
    uint m_progressSteps, m_progressTotalSteps, m_lastProgressPerCent;
};

}

#endif
