/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DLG_CLONESARRAY
#define DLG_CLONESARRAY

#include <KoDialog.h>
#include <kis_global.h>
#include <QPointer>

#include "kis_types.h"
#include "ui_wdg_clonesarray.h"

class KisViewManager;
class KisProcessingApplicator;


class WdgClonesArray : public QWidget, public Ui::WdgClonesArray
{
    Q_OBJECT

public:
    WdgClonesArray(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class DlgClonesArray: public KoDialog
{

    Q_OBJECT

public:
    DlgClonesArray(KisViewManager *viewManager, QWidget * parent = 0);
    ~DlgClonesArray() override;

private Q_SLOTS:
    void okClicked();
    void applyClicked();
    void cancelClicked();

    void syncOrthogonalToAngular();
    void syncAngularToOrthogonal();

    void setDirty();
    void updateCheckboxAvailability();

private:
    void setOrthogonalSignalsEnabled(bool value);
    void setAngularSignalsEnabled(bool value);

    void initializeValues();
    void reapplyClones();

    void setClean();

private:
    WdgClonesArray *m_page;
    QPointer<KisViewManager> m_viewManager;
    KisProcessingApplicator *m_applicator;
    KisLayerSP m_baseLayer;
    bool m_isDirty;
};

#endif // DLG_CLONESARRAY
