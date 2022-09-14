/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGCOLORSELECTORSETTINGS_H
#define WGCOLORSELECTORSETTINGS_H

#include "WGConfig.h"

#include <QDialog>
#include <QScopedPointer>

#include "kis_preference_set_registry.h"
#include "KisColorSelectorConfiguration.h"

namespace Ui {
    class WGConfigWidget;
}
class WGSelectorConfigGrid;
class WGShadeLineEditor;
class QButtonGroup;
class QToolButton;

class WGColorSelectorSettings : public KisPreferenceSet
{
    Q_OBJECT
public:
    explicit WGColorSelectorSettings(QWidget *parent = 0);
    ~WGColorSelectorSettings() override;

    QString id() override;
    QString name() override;
    QString header() override;
    QIcon icon() override;

    static QString stringID();
public Q_SLOTS:
    void savePreferences() const override;
    void loadPreferences() override;
    void loadDefaultPreferences() override;
private:
    void loadPreferencesImpl(bool loadDefaults);

private Q_SLOTS:
    void slotSetSelectorConfiguration(const KisColorSelectorConfiguration &cfg);
    void slotSetColorModel(int index);
    void slotColorSpaceSourceChanged(int index);
    void slotSetShadeLineCount(int count);
    void slotShowLineEditor(int lineNum);
    void slotLineEdited(int lineNum);
private:
    QScopedPointer<Ui::WGConfigWidget> m_ui;
    WGSelectorConfigGrid *m_selectorConfigGrid;
    WGSelectorConfigGrid *m_favoriteConfigGrid;
    WGShadeLineEditor *m_shadeLineEditor;
    QButtonGroup *m_shadeLineGroup;
    QVector<WGConfig::ShadeLine> m_shadeLineConfig;
    QVector<QToolButton*> m_shadeLineButtons;
};


class WGColorSelectorSettingsFactory : public KisAbstractPreferenceSetFactory
{
public:
    KisPreferenceSet* createPreferenceSet() override
    {
        return new WGColorSelectorSettings();
    }
    QString id() const override { return "WGColorSelectorSettings"; }
};

class WGColorSelectorSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WGColorSelectorSettingsDialog(QWidget *parent = 0);
private:
    WGColorSelectorSettings* m_widget;
};

#endif // WGCOLORSELECTORSETTINGS_H
