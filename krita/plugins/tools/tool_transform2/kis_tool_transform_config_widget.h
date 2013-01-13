/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_TOOL_TRANSFORM_CONFIG_WIDGET_H
#define __KIS_TOOL_TRANSFORM_CONFIG_WIDGET_H

#include "transform_transaction_properties.h"
#include "tool_transform_args.h"
#include "ui_wdg_tool_transform.h"

class KisCanvas2;


class KisToolTransformConfigWidget : public QWidget, public Ui::WdgToolTransform
{
    Q_OBJECT

public:
    KisToolTransformConfigWidget(const TransformTransactionProperties *transaction, KisCanvas2 *canvas, QWidget *parent);

    void setApplyResetDisabled(bool disabled);
    void resetRotationCenterButtons();

public slots:
    void updateConfig(const ToolTransformArgs &config);

signals:
    void sigConfigChanged();

private slots:
/*    void slotFilterChanged(const KoID &filter);
    void slotWarpTypeChanged(int index);

*/
    void slotRotationCenterChanged(int index);

    void slotSetScaleX(double value);
    void slotSetScaleY(double value);

    void slotSetShearX(double value);
    void slotSetShearY(double value);

    void slotSetTranslateX(double value);
    void slotSetTranslateY(double value);

    void slotSetAX(double value);
    void slotSetAY(double value);
    void slotSetAZ(double value);

    void slotSetWrapAlpha(double value);
    void slotSetWrapDensity(int value);

    void slotSetKeepAspectRatio(bool value);
/*
    void slotWarpDefaultPointsButtonClicked(bool value);
    void slotWarpCustomPointsButtonClicked(bool value);
    void slotWarpLockPointsButtonClicked();
    void slotWarpResetPointsButtonClicked(bool value);

    void slotSetFreeTransformModeButtonClicked(bool);
    void slotSetWrapModeButtonClicked(bool);

    void slotEditingFinished();

    void slotButtonBoxClicked();
*/

private:
    // rad being in |R, the returned value is in [0; 360]
    double radianToDegree(double rad);
    // degree being in |R, the returned value is in [0; 2*M_PI]
    double degreeToRadian(double degree);

    void blockNotifications();
    void unblockNotifications();
    void notifyConfigChanged();

    void blockUiSlots();
    void unblockUiSlots();

private:
    /**
     * This is a link to some additional data about transformation.
     * The constness is added intentively. The values inside should be
     * changed by means of signals
     */
    const TransformTransactionProperties *m_transaction;
    QPointF m_handleDir[9];
    QButtonGroup *m_rotationCenterButtons;
    int m_notificationsBlocked;
    int m_uiSlotsBlocked;
//    bool m_configChanged;
//    ToolTransformArgs m_currentConfig;
};

#endif /* __KIS_TOOL_TRANSFORM_CONFIG_WIDGET_H */
