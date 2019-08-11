/*
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
