/*
 *  Copyright (c) 2020 Halla Rempt <halla@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISPREVIEWFILEDIALOG_H
#define KISPREVIEWFILEDIALOG_H

#include <QFileDialog>
#include <QFileIconProvider>

#include <kritawidgetutils_export.h>

class QLabel;
class QToolButton;

class KisFileIconProvider : public QFileIconProvider
{
public:

    KisFileIconProvider(qreal devicePixelRatioF);
    QIcon icon(const QFileIconProvider::IconType type) const override;
    QIcon icon(const QFileInfo &fi) const override;
private:

    qreal m_devicePixelRatioF;

};

class KRITAWIDGETUTILS_EXPORT KisAbstractFileIconCreator
{
public:

    KisAbstractFileIconCreator();
    virtual ~KisAbstractFileIconCreator();

    virtual bool createFileIcon(QString path, QIcon &icon, qreal devicePixelRatioF, QSize iconSize) = 0;

};

class KRITAWIDGETUTILS_EXPORT KisPreviewFileDialog : public QFileDialog
{
    Q_OBJECT

public:
    KisPreviewFileDialog(QWidget *parent = nullptr, const QString &caption = QString(), const QString &directory = QString(), const QString &filter = QString());

    // QFileDialog drops the icon provider when the model is created,
    // so we cannot set this before the options are set
    void resetIconProvider();

    static KisAbstractFileIconCreator *s_iconCreator;

public Q_SLOTS:

    void onCurrentChanged(const QString &path);
    void previewToggled(bool showPreview);

private:

    KisFileIconProvider *m_iconProvider {0};
    QLabel *m_preview {0};
    QToolButton *m_previewToggle {0};
};

#endif // KISPREVIEWFILEDIALOG_H
