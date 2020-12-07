/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISRECTANGLECONSTRAINTWIDGET_H
#define KISRECTANGLECONSTRAINTWIDGET_H

#include "ui_wdgrectangleconstraints.h"
#include <kritaui_export.h>

class KisToolRectangleBase;
class KisAspectRatioLocker;

class KRITAUI_EXPORT KisRectangleConstraintWidget : public QWidget, public Ui::WdgRectangleConstraints
{
  Q_OBJECT
  
public:
    KisRectangleConstraintWidget(QWidget *parentWidget, KisToolRectangleBase *tool, bool showRoundCornersGUI);
    
Q_SIGNALS:
  void constraintsChanged(bool forceRatio, bool forceWidth, bool forceHeight, float ratio, float width, float height);
  
protected Q_SLOTS:
  void rectangleChanged(const QRectF &rect);
  void inputsChanged();

  void slotRoundCornersChanged();
  void slotRoundCornersAspectLockChanged();

  void slotReloadConfig();
  
protected:
  KisToolRectangleBase* m_tool;
  Ui_WdgRectangleConstraints *m_widget;
  KisAspectRatioLocker *m_cornersAspectLocker;
};

#endif
