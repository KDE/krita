/*
 *  SPDX-FileCopyrightText: 2010 Celarek Adam <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    void useCustomColorForSelector(bool);
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
