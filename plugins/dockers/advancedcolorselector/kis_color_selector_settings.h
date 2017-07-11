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

#ifndef KIS_COLOR_SELECTOR_SETTINGS_H
#define KIS_COLOR_SELECTOR_SETTINGS_H


#include <QDialog>
#include "kis_preference_set_registry.h"

namespace Ui {
    class KisColorSelectorSettings;
}
class QIcon;

class KisColorSelectorSettings : public KisPreferenceSet {
    Q_OBJECT
public:
    KisColorSelectorSettings(QWidget *parent = 0);
    ~KisColorSelectorSettings() override;

    QString id() override;
    QString name() override;
    QString header() override;
    QIcon icon() override;

public Q_SLOTS:
    void savePreferences() const override;
    void loadPreferences() override;
    void loadDefaultPreferences() override;

    void changedColorDocker(int);
    void useDifferentColorSpaceChecked(bool);
    void changedACSColorSelectorType(int);
    void changedACSShadeSelectorType(int);
    void changedACSColorAlignment(bool);
    void changedACSLastUsedColorAlignment(bool);

Q_SIGNALS:
    void settingsChanged() const;
    void hsxchanged(int);

protected:
    //void changeEvent(QEvent *e);

private:
    Ui::KisColorSelectorSettings *ui;
};

class KisColorSelectorSettingsUpdateRepeater : public QObject {
    Q_OBJECT
Q_SIGNALS:
    void settingsUpdated();
public Q_SLOTS:
    void updateSettings() {
        emit settingsUpdated();
    }
};

class KisColorSelectorSettingsFactory : public KisAbstractPreferenceSetFactory {
public:
    KisPreferenceSet* createPreferenceSet() override {
        KisColorSelectorSettings* ps = new KisColorSelectorSettings();
        QObject::connect(ps, SIGNAL(settingsChanged()), &repeater, SLOT(updateSettings()), Qt::UniqueConnection);
        return ps;
    }
    QString id() const override { return "ColorSelectorSettings"; }
    KisColorSelectorSettingsUpdateRepeater repeater;
};

class KisColorSelectorSettingsDialog : public QDialog {
Q_OBJECT
public:
    KisColorSelectorSettingsDialog(QWidget *parent = 0);
private:
    KisColorSelectorSettings* m_widget;
};


#endif // KIS_COLOR_SELECTOR_SETTINGS_H
