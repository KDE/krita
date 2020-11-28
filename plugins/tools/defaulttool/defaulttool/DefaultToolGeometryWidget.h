/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Martin Pfeiffer <hubipete@gmx.net>
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2010 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
