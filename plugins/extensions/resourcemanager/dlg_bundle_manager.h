/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
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
#ifndef DLG_BUNDLE_MANAGER_H
#define DLG_BUNDLE_MANAGER_H

#include <QWidget>
#include <KoDialog.h>

class KisStorageModel;

namespace Ui
{
class WdgDlgBundleManager;
}

class DlgBundleManager : public KoDialog
{
    Q_OBJECT
public:
    explicit DlgBundleManager(QWidget *parent = 0);

private Q_SLOTS:

    void addBundle();
    void createBundle();
    void deleteBundle();

private:

    QWidget *m_page;
    Ui::WdgDlgBundleManager *m_ui;
};

#endif // DLG_BUNDLE_MANAGER_H
