/*
 *  Copyright (C) 2010 Celarek Adam <kdedev at xibo dot at>
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

#ifndef KIS_COLSELNG_SETTINGS_H
#define KIS_COLSELNG_SETTINGS_H


#include <QDialog>
#include "kis_preference_set_registry.h"

namespace Ui {
    class KisColorSelectorSettings;
}
class KIcon;
class KisCanvas2;


class KisColorSelectorSettings : public KisPreferenceSet {
    Q_OBJECT
public:
    KisColorSelectorSettings(QWidget *parent = 0);
    ~KisColorSelectorSettings();

    virtual QString id();
    virtual QString name();
    virtual QString header();
    virtual KIcon icon();

public slots:
    void savePreferences() const;
    void loadPreferences();

protected:
//    void changeEvent(QEvent *e);

private:
    Ui::KisColorSelectorSettings *ui;
};

class KisColorSelectorSettingsDialog : public QDialog {
public:
    KisColorSelectorSettingsDialog(QWidget *parent = 0);
private:
    KisColorSelectorSettings* m_widget;
};


#endif // KIS_COLSELNG_SETTINGS_H
