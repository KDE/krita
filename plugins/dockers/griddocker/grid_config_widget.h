/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef GRID_CONFIG_WIDGET_H
#define GRID_CONFIG_WIDGET_H

#include <QWidget>
#include <QScopedPointer>

namespace Ui {
class GridConfigWidget;
}

class KisGridConfig;
class KisGuidesConfig;

class GridConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GridConfigWidget(QWidget *parent = 0);
    ~GridConfigWidget() override;

    void setGridConfig(const KisGridConfig &value);
    KisGridConfig gridConfig() const;

    void setGuidesConfig(const KisGuidesConfig &value);
    KisGuidesConfig guidesConfig() const;

    bool showRulers() const;

    void enableIsometricGrid(bool value);

public Q_SLOTS:
    void setShowRulers(bool value);

private Q_SLOTS:
    void slotGridGuiChanged();
    void slotGuidesGuiChanged();
    void slotGridTypeChanged();
    void slotPreferencesUpdated();

Q_SIGNALS:
    void gridValueChanged();
    void guidesValueChanged();
    void showRulersChanged(bool);

private:
    KisGridConfig fetchGuiGridConfig() const;
    void setGridConfigImpl(const KisGridConfig &value);

    KisGuidesConfig fetchGuiGuidesConfig() const;
    void setGuidesConfigImpl(const KisGuidesConfig &value);


private:
    Ui::GridConfigWidget *ui;

    struct Private;
    const QScopedPointer<Private> m_d;
    bool m_isGridEnabled {false};

    bool m_isIsometricGridEnabled {true};
};

#endif // GRID_CONFIG_WIDGET_H
