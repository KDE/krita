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
    bool m_isDirty {false};
};

#endif // DLG_CLONESARRAY
