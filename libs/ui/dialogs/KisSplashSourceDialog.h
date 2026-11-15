// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef KIS_SPLASH_SOURCE_DIALOG_H
#define KIS_SPLASH_SOURCE_DIALOG_H
#include "ui_KisSplashSourceDialog.h"
#include <QDialog>
#include <QVector>
#include <kis_splash_screen.h>

class KisSignalCompressor;

class KisSplashSourceDialog : public QDialog, public Ui::KisSplashSourceDialog
{
    Q_OBJECT
public:
    explicit KisSplashSourceDialog(const QString &key, QWidget *parent = nullptr);

    QString selectedKey() const;

private Q_SLOTS:
    void slotSourceSelected();
    void slotUpdatePreview();
    void slotRefreshSources();
#ifdef Q_OS_ANDROID
    void slotShowDonationDialog();
#endif

private:
    void initSources(const QString &key);
    const KisSplashScreen::Source *currentSource() const;

    static QString makePurchaseOption(const QString &key, const QString &text);

    QVector<KisSplashScreen::Source> m_sources;
    KisSignalCompressor *m_updatePreviewCompressor;
};

#endif
