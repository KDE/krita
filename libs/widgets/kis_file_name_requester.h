/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_FILE_NAME_REQUESTER_H
#define KIS_FILE_NAME_REQUESTER_H

#include "kritawidgets_export.h"

#include <QWidget>
#include <QString>
#include <QUrl>
#include <KoFileDialog.h>


namespace Ui {
    class WdgFileNameRequester;
}

/**
 * This represents an editable file name.
 * Visual it presents a QLineEdit + a button that pops up
 * a file chooser.
 *
 * Signals are fired when the user changes the text
 * or selects a new file via the button/file chooser.
 */
class KRITAWIDGETS_EXPORT KisFileNameRequester : public QWidget
{
    Q_OBJECT

public:
    explicit KisFileNameRequester(QWidget *parent = 0);
    ~KisFileNameRequester() override;

    void setStartDir(const QString &path);
    /// Set the name used to store the last-used directory in the settings
    void setConfigurationName(const QString &name);

    QString fileName() const;
    void setFileName(const QString &path);

    void setMode(KoFileDialog::DialogType mode);
    KoFileDialog::DialogType mode() const;

    /**
     * Sets the mime type filters to use, same format as KoFileDialog::setMimeTypeFilters.
     * If this is not called, the default list is used, which simply selects all the image
     * file formats Krita can load.
     */
    void setMimeTypeFilters(const QStringList &filterList, QString defaultFilter = QString());

public Q_SLOTS:
    void slotSelectFile();

Q_SIGNALS:
    void textChanged(const QString &fileName);
    void fileSelected(const QString &fileName);

private:
    QScopedPointer<Ui::WdgFileNameRequester> m_ui;
    QString m_basePath;
    KoFileDialog::DialogType m_mode;
    QStringList m_mime_filter_list;
    QString m_mime_default_filter;
    QString m_name;
};

#endif // KIS_FILE_NAME_REQUESTER_H
