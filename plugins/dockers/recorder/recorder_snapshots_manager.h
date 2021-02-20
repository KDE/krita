/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
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
