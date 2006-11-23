/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 *
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
#ifndef KIS_MASK_MANAGER
#define KIS_MASK_MANAGER

#include <QObject>

class KisView2;
class KActionCollection;
class KAction;
class KToggleAction;

class KisMaskManager : public QObject {

    Q_OBJECT

public:


    KisMaskManager(KisView2 * view );
    ~KisMaskManager() {}

    void setup(KActionCollection * actionCollection);
    void updateGUI();

public slots:

    void slotCreateMask();
    void slotMaskFromSelection();
    void slotMaskToSelection();
    void slotApplyMask();
    void slotRemoveMask();
    void slotEditMask();
    void slotShowMask();
    void maskUpdated();

private:
    KisView2 * m_view;

    KAction *m_createMask;
    KAction *m_maskFromSelection;
    KAction *m_maskToSelection;
    KAction *m_applyMask;
    KAction *m_removeMask;
    KToggleAction *m_editMask;
    KToggleAction *m_showMask;

};

#endif // KIS_MASK_MANAGER
