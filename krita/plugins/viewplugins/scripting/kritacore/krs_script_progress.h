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

#ifndef KROSS_KRITACOREKRS_SCRIPTPROGRESS_H
#define KROSS_KRITACOREKRS_SCRIPTPROGRESS_H

#include <api/class.h>

class KisScriptProgress;

namespace Kross {

namespace KritaCore {

/**
 * ScriptProgress is used to manage the progress bar of the status bar in krita
 * 
 * For example (in ruby) :
 * @code
 * script = Krosskritacore::get("KritaScript")
 * script.setProgressTotalSteps(1000)
 * script.setProgressStage("progressive", 0)
 * for i in 1..900
 *   script.incProgress()
 * end
 * script.setProgressStage("brutal", 1000)
 * @endcode
 */
class ScriptProgress : public Kross::Api::Class<ScriptProgress> {
    public:
        ScriptProgress(KisScriptProgress* Script);
        ~ScriptProgress();
    private:
        /**
         * This function set the number of steps that the script will require.
         * It takes one argument :
         * - maximum value of the progress
         */
        Kross::Api::Object::Ptr setProgressTotalSteps(Kross::Api::List::Ptr);
        /**
         * This function set the value of progress.
         * It takes one argument :
         * - value of the progress
         */
        Kross::Api::Object::Ptr setProgress(Kross::Api::List::Ptr);
        /**
         * This function increment of one step the position of the progress.
         */
        Kross::Api::Object::Ptr incProgress(Kross::Api::List::Ptr);
        /**
         * This function set the value of the progress and display the text 
         */
        Kross::Api::Object::Ptr setProgressStage(Kross::Api::List::Ptr);
    private:
        KisScriptProgress* m_script;
};

}

}

#endif
