/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TOOL_TRANSFORM_CONFIG_WIDGET_H
#define __KIS_TOOL_TRANSFORM_CONFIG_WIDGET_H

#include "transform_transaction_properties.h"
#include "tool_transform_args.h"
#include "ui_wdg_tool_transform.h"

class KisCanvas2;


class KisToolTransformConfigWidget : public QWidget, private Ui::WdgToolTransform
{
    Q_OBJECT

public:
    KisToolTransformConfigWidget(TransformTransactionProperties *transaction, KisCanvas2 *canvas, bool workRecursively, QWidget *parent);

    void setApplyResetDisabled(bool disabled);
    void resetRotationCenterButtons();
    void setDefaultWarpPoints(int pointsPerLine = -1);
    void setTooBigLabelVisible(bool value);
    bool workRecursively() const;

public Q_SLOTS:
    void updateConfig(const ToolTransformArgs &config);
    void slotUpdateIcons();

Q_SIGNALS:
    void sigConfigChanged();
    void sigApplyTransform();
    void sigResetTransform(ToolTransformArgs::TransformMode mode);
    void sigCancelTransform();
    void sigRestartTransform();
    void sigEditingFinished();

public Q_SLOTS:

    void slotFilterChanged(const KoID &filter);
    void slotWarpTypeChanged(int index);
    void slotRotationCenterChanged(int index);
    void slotTransformAroundRotationCenter(bool value);

    void slotSetScaleX(int value);
    void slotSetScaleY(int value);

    void slotSetShearX(qreal value);
    void slotSetShearY(qreal value);

    void slotSetTranslateX(int value);
    void slotSetTranslateY(int value);

    void slotSetAX(qreal value);
    void slotSetAY(qreal value);
    void slotSetAZ(qreal value);

    void slotFlipX();
    void slotFlipY();
    void slotRotateCW();
    void slotRotateCCW();

    void slotSetWarpAlpha(qreal value);
    void slotSetWarpDensity(int value);

    void slotSetKeepAspectRatio(bool value);

    void slotTransformAreaVisible(bool value);

    void slotWarpDefaultPointsButtonClicked(bool value);
    void slotWarpCustomPointsButtonClicked(bool value);
    void slotWarpLockPointsButtonClicked();
    void slotWarpResetPointsButtonClicked();

    void slotSetFreeTransformModeButtonClicked(bool);
    void slotSetWarpModeButtonClicked(bool);
    void slotSetCageModeButtonClicked(bool);
    void slotCageOptionsChanged(int);

    void slotSetPerspectiveModeButtonClicked(bool);
    void slotSetLiquifyModeButtonClicked(bool);
    void slotButtonBoxClicked(QAbstractButton *button);

    void slotSetMeshModeButtonClicked(bool);


    void slotEditCagePoints(bool value);

    void liquifySizeChanged(qreal value);
    void liquifyAmountChanged(qreal value);
    void liquifyFlowChanged(qreal value);
    void liquifyBuildUpChanged(int value);
    void liquifySpacingChanged(qreal value);
    void liquifySizePressureChanged(bool value);
    void liquifyAmountPressureChanged(bool value);
    void liquifyReverseDirectionChanged(bool value);

    void slotLiquifyModeChanged(int value);

    void notifyEditingFinished();

    void slotGranularityChanged(QString value);
    void slotPreviewGranularityChanged(QString value);

    void slotMeshSizeChanged();
    void slotMeshShowHandlesChanged();
    void slotMeshSymmetricalHandlesChanged();
    void slotMeshScaleHandlesChanged();
private:

    void blockNotifications();
    void unblockNotifications();
    void notifyConfigChanged();

    void blockUiSlots();
    void unblockUiSlots();

    void activateCustomWarpPoints(bool enabled);

    void updateLockPointsButtonCaption();

    void updateLiquifyControls();

    void resetUIOptions();

private:
    static const int DEFAULT_POINTS_PER_LINE;

private:
    TransformTransactionProperties *m_transaction;
    QPointF m_handleDir[9];
    QButtonGroup *m_rotationCenterButtons;
    int m_notificationsBlocked;
    int m_uiSlotsBlocked;
    double m_scaleRatio;
    bool m_configChanged;
};

#endif /* __KIS_TOOL_TRANSFORM_CONFIG_WIDGET_H */
