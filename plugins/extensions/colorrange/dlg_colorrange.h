/*
 *  dlg_colorrange.h -- part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef DLG_COLORRANGE
#define DLG_COLORRANGE

#include <QCursor>

#include <KoDialog.h>

#include <kis_selection.h> // For enums
#include <kis_pixel_selection.h>
#include <kis_types.h>
#include <kis_global.h>

#include "ui_wdg_colorrange.h"

class KisViewManager;
class DlgColorRange;

enum enumAction {
    REDS,
    YELLOWS,
    GREENS,
    CYANS,
    BLUES,
    MAGENTAS,
    HIGHLIGHTS,
    MIDTONES,
    SHADOWS
};

class WdgColorRange : public QWidget, public Ui::WdgColorRange
{
    Q_OBJECT

public:
    WdgColorRange(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
* This dialog allows the user to create a selection mask based
* on a (range of) colors.
*/
class DlgColorRange: public KoDialog
{

    Q_OBJECT

public:

    DlgColorRange(KisViewManager *viewManager, QWidget *parent = 0);
    ~DlgColorRange() override;

private Q_SLOTS:

    void okClicked();
    void cancelClicked();

    void slotInvertClicked();
    void slotSelectionTypeChanged(int index);
    void slotSubtract(bool on);
    void slotAdd(bool on);
    void slotSelectClicked();
    void slotDeselectClicked();

private:
    QImage createMask(KisSelectionSP selection, KisPaintDeviceSP layer);

private:

    WdgColorRange *m_page;
    int m_selectionCommandsAdded;
    KisViewManager *m_viewManager;
    SelectionAction m_mode;
    QCursor m_oldCursor;
    enumAction m_currentAction;
    bool m_invert;
};


#endif // DLG_COLORRANGE
