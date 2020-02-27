/*
 *  Copyright (c) 2020 Anna Medonosová <anna.medonosova@gmail.com>
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

#ifndef KISSPENSETTINGS_H
#define KISSPENSETTINGS_H

#include <QObject>
#include <QString>
#include <QStandardItemModel>
#include <kis_preference_set_registry.h>

#include "ui_wdg_spensettings.h"

class QModelIndex;

class WdgSPenSettings : public QWidget, public Ui::WdgSPenSettings
{
    Q_OBJECT

public:
    WdgSPenSettings(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};


class KisSPenSettings : public KisPreferenceSet
{
    Q_OBJECT
public:
    KisSPenSettings(QWidget* parent = 0);
    ~KisSPenSettings() override;

    QString id() override;
    QString name() override;
    QString header() override;
    QIcon icon() override;

public Q_SLOTS:
    void savePreferences() const override;
    void loadPreferences() override;
    void loadDefaultPreferences() override;

Q_SIGNALS:
    void settingsChanged() const;

private:
    static const int m_ACTION_TEXT_COLUMN = 0;
    static const int m_ACTION_NAME_COLUMN = 1;

    QString actionNameForIndex(int index) const;
    int indexFromActionName(QString actionName) const;

    WdgSPenSettings* mUi;
    QStandardItemModel* m_model;
};

class KisSPenSettingsUpdateRepeater : public QObject {
    Q_OBJECT
Q_SIGNALS:
    void settingsUpdated();
public Q_SLOTS:
    void updateSettings() {
        emit settingsUpdated();
    }
};

class KisSPenSettingsFactory : public KisAbstractPreferenceSetFactory {
public:
    KisPreferenceSet* createPreferenceSet() override {
        KisSPenSettings* ps = new KisSPenSettings();
        QObject::connect(ps, SIGNAL(settingsChanged()), &repeater, SLOT(updateSettings()), Qt::UniqueConnection);
        return ps;
    }
    QString id() const override { return "SPenSettings"; }
    KisSPenSettingsUpdateRepeater repeater;
};


#endif // KISSPENSETTINGS_H
