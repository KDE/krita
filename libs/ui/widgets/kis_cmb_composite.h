/*
 *  widgets/kis_cmb_composite.h - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#ifndef KIS_COMPOSITEOP_WIDGETS_H_
#define KIS_COMPOSITEOP_WIDGETS_H_

#include <QComboBox>
#include <KisSqueezedComboBox.h>
#include <kritaui_export.h>
#include "kis_categorized_list_view.h"

class KoID;
class KoColorSpace;
class KisSortedCompositeOpListModel;
class KisAction;

class KRITAUI_EXPORT KisCompositeOpListWidget: public KisCategorizedListView
{
public:
     KisCompositeOpListWidget(QWidget* parent=0);
    ~KisCompositeOpListWidget() override;

    KoID selectedCompositeOp() const;

private:
    KisSortedCompositeOpListModel *m_model;
};


class KRITAUI_EXPORT KisCompositeOpComboBox: public KisSqueezedComboBox
{
    Q_OBJECT
public:
     KisCompositeOpComboBox(QWidget* parent=0);
    ~KisCompositeOpComboBox() override;

    void hidePopup() override;

    void validate(const KoColorSpace *cs);
    void selectCompositeOp(const KoID &op);
    KoID selectedCompositeOp() const;

    QList<KisAction *> blendmodeActions() const;

private Q_SLOTS:
    void slotCategoryToggled(const QModelIndex& index, bool toggled);
    void slotEntryChecked(const QModelIndex& index);

    void slotNextBlendingMode();
    void slotPreviousBlendingMode();
    void slotNormal();
    void slotDissolve();
    void slotBehind();
    void slotClear();
    void slotDarken();
    void slotMultiply();
    void slotColorBurn();
    void slotLinearBurn();
    void slotLighten();
    void slotScreen();
    void slotColorDodge();
    void slotLinearDodge();
    void slotOverlay();
    void slotHardOverlay();
    void slotSoftLight();
    void slotHardLight();
    void slotVividLight();
    void slotLinearLight();
    void slotPinLight();
    void slotHardMix();
    void slotDifference();
    void slotExclusion();
    void slotHue();
    void slotSaturation();
    void slotColor();
    void slotLuminosity();



private:
    KisSortedCompositeOpListModel *m_model;
    KisCategorizedListView *m_view;
    bool m_allowToHidePopup;
    QList<KisAction *> m_actions;
};

#endif // KIS_COMPOSITEOP_WIDGETS_H_
