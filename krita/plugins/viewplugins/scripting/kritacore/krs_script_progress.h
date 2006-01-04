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

class ScriptProgress : public Kross::Api::Class<ScriptProgress> {
    public:
        ScriptProgress(KisScriptProgress* Script);
        ~ScriptProgress();
    private:
        Kross::Api::Object::Ptr setProgressTotalSteps(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr setProgress(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr incProgress(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr setProgressStage(Kross::Api::List::Ptr);
    private:
        KisScriptProgress* m_script;
};

}

}

#endif
