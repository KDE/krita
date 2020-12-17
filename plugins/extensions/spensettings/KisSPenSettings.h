/*
 *  SPDX-FileCopyrightText: 2020 Anna Medonosová <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
