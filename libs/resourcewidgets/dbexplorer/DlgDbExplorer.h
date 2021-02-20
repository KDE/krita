/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DLG_DBEXPLORER
#define DLG_DBEXPLORER

#include <KoDialog.h>

#include "ui_WdgDbExplorer.h"
//#include <KisTagFilterResourceProxyModel.h>

class KisResourceModel;
class KisTagModel;
class KisResourceTypeModel;
class KisTagFilterResourceProxyModel;


class WdgDbExplorer : public QWidget, public Ui::WdgDbExplorer
{
    Q_OBJECT

public:
    WdgDbExplorer(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class DlgDbExplorer: public KoDialog
{
    Q_OBJECT
public:
    DlgDbExplorer(QWidget * parent = 0);
    ~DlgDbExplorer() override;

private Q_SLOTS:

    void slotTbResourceTypeSelected(int index);
    void slotTbResourceItemSelected();

    void slotRvResourceTypeSelected(int index);
    void slotRvTagSelected(int index);

private:
    void updateTagModel(const QString& resourceType);

    WdgDbExplorer *m_page {0};

    KisTagModel *m_tagModel {0};
    KisResourceTypeModel *m_resourceTypeModel {0};
    KisTagFilterResourceProxyModel* m_filterProxyModel {0};
};

#endif // DLG_DBEXPLORER
