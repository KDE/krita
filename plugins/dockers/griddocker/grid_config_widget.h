/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

    void setGridDivision(int w, int h);

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
    bool m_isGridEnabled;

    bool m_isIsometricGridEnabled = true;
};

#endif // GRID_CONFIG_WIDGET_H
