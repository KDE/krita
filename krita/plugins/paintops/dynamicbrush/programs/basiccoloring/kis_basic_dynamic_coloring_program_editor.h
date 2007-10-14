/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_BASIC_DYNAMIC_COLORING_PROGRAM_EDITOR_H_
#define _KIS_BASIC_DYNAMIC_COLORING_PROGRAM_EDITOR_H_

#include <QWidget>

class Ui_BasicDynamicColoringProgramEditor;

class KisBasicDynamicColoringProgram;

class KisBasicDynamicColoringProgramEditor : public QWidget {
    Q_OBJECT
    public:
        KisBasicDynamicColoringProgramEditor(KisBasicDynamicColoringProgram* program);
        ~KisBasicDynamicColoringProgramEditor();
    public slots:
        void setMixerEnable(bool v);
        void setHueEnable(bool v);
        void setSaturationEnable(bool v);
        void setBrightnessEnable(bool v);
    private:
        Ui_BasicDynamicColoringProgramEditor* m_basicDynamicColoringProgramEditor;
        KisBasicDynamicColoringProgram* m_program;
};

#endif
