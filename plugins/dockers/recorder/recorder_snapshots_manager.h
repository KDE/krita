/*
 *  Copyright (c) 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef WIDGET_H
#define WIDGET_H

#include "recorder_snapshots_scanner.h"

#include <QDialog>

namespace Ui {
class RecorderSnapshotsManager;
}

class QItemSelection;
class RecorderDirectoryCleaner;

class RecorderSnapshotsManager : public QDialog
{
    Q_OBJECT

public:
    RecorderSnapshotsManager(QWidget *parent = nullptr);
    ~RecorderSnapshotsManager();

    void execFor(const QString &snapshotsDirectory);

protected:
    void closeEvent(QCloseEvent *event) override;

private Q_SLOTS:
    void reject() override;

    void onScanningFinished(SnapshotDirInfoList snapshots);
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onButtonSelectAllClicked();
    void onButtonCleanUpClicked();
    void onButtonCancelCleanUpClicked();
    void onCleanUpFinished();

private:
    void startScanning();
    void updateSpaceToBeFreed();
    void abortCleanUp();

private:
    Ui::RecorderSnapshotsManager *ui;
    RecorderSnapshotsScanner *scanner;
    RecorderDirectoryCleaner* cleaner;
};

#endif // WIDGET_H
