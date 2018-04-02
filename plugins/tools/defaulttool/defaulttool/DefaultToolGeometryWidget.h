/* This file is part of the KDE project
 * Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef DEFAULTTOOLGEOMETRYWIDGET_H
#define DEFAULTTOOLGEOMETRYWIDGET_H

#include <ui_DefaultToolGeometryWidget.h>
#include <KoFlake.h>

#include <QWidget>


class KoInteractionTool;
class KisAspectRatioLocker;

class DefaultToolGeometryWidget : public QWidget, Ui::DefaultToolGeometryWidget
{
    Q_OBJECT
public:
    explicit DefaultToolGeometryWidget(KoInteractionTool *tool, QWidget *parent = 0);
    ~DefaultToolGeometryWidget() override;

    /// Sets the unit used by the unit aware child widgets
    void setUnit(const KoUnit &unit);

    bool useUniformScaling() const;

protected:
    void showEvent(QShowEvent *event) override;

private Q_SLOTS:
    void slotAnchorPointChanged();
    void resourceChanged(int key, const QVariant &res);

    void slotUpdatePositionBoxes();
    void slotRepositionShapes();

    void slotUpdateSizeBoxes(bool updateAspect = true);
    void slotUpdateSizeBoxesNoAspectChange();
    void slotResizeShapes();

    void slotUpdateCheckboxes();

    void slotAspectButtonToggled();
    void slotUpdateAspectButton();

    void slotOpacitySliderChanged(qreal newOpacity);
    void slotUpdateOpacitySlider();

private:
    KoInteractionTool *m_tool;
    QScopedPointer<KisAspectRatioLocker> m_sizeAspectLocker;
    bool m_savedUniformScaling;
};

#endif
