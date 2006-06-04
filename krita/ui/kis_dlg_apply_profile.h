/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_DLG_APPLY_PROFILE_H_
#define KIS_DLG_APPLY_PROFILE_H_

#include <kdialogbase.h>

#include "ui_wdgapplyprofile.h"

class KoID;

class WdgApplyProfile : public QWidget, public Ui::WdgApplyProfile
{
    Q_OBJECT

    public:
        WdgApplyProfile(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

class KisDlgApplyProfile : public KDialog {
    typedef KDialog super;

    Q_OBJECT

public:
    KisDlgApplyProfile(QWidget *parent = 0,
               const char *name = 0);
    virtual ~KisDlgApplyProfile();


    KoColorProfile *  profile() const;
    int renderIntent() const;

    void fillCmbProfiles(const KoID & s);

private:
    QButtonGroup *m_intentButtonGroup;
    WdgApplyProfile * m_page;
};

#endif // KIS_DLG_APPLY_PROFILE_H_

