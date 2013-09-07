/* This file is part of the KDE project
   Copyright (C) 2013 Yue Liu <yue.liu@mail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOFILEDIALOG_H
#define KOFILEDIALOG_H

#include "komain_export.h"

#include <QFileDialog>
#include <QString>
#include <QStringList>

/**
 * Wrapper around QFileDialog providing native file dialogs
 * on KDE/Gnome/Qt/Windows/OSX
 */
class KOMAIN_EXPORT KoFileDialog : QObject
{
    Q_OBJECT
public:
    KoFileDialog() {}
    ~KoFileDialog() {}

    /**
     * @brief getOpenFileName - used for open an existing document
     * @param parent The parent of the file dialog
     * @param caption Caption of the file dialog
     * @param dir Directory when file dialog appear
     * @param mimeList List of mimetypes user can select in the file dialog
     * @param selectedMime Mimetype selected by default
     * @return The name of the entry user selected in the file dialog
     */
    QString getOpenFileName(QWidget *parent = 0,
                            const QString &caption = QString(),
                            const QString &dir = QString(),
                            const QStringList &mimeList = QStringList(),
                            const QString &selectedMime = 0);

    QStringList getOpenFileNames(QWidget *parent = 0,
                                 const QString &caption = QString(),
                                 const QString &dir = QString(),
                                 const QStringList &mimeList = QStringList(),
                                 const QString &selectedMime = 0);

    QString getOpenDirectory(QWidget *parent = 0,
                             const QString &caption = QString(),
                             const QString &dir = QString());
    /**
     * @brief getImportFileName - used for import stuff to current document
     * These will be sheet dialogs under OSX
     * @param parent The parent of the file dialog
     * @param caption Caption of the file dialog
     * @param dir Directory when file dialog appear
     * @param mimeList List of mimetypes user can select in the file dialog
     * @param selectedMime Mimetype selected by default
     * @return The name of the entry user selected in the file dialog
     */
    QString getImportFileName(QWidget *parent = 0,
                              const QString &caption = QString(),
                              const QString &dir = QString(),
                              const QStringList &mimeList = QStringList(),
                              const QString &selectedMime = 0);

    QStringList getImportFileNames(QWidget *parent = 0,
                               const QString &caption = QString(),
                               const QString &dir = QString(),
                               const QStringList &mimeList = QStringList(),
                               const QString &selectedMime = 0);

    QString getImportDirectory(QWidget *parent = 0,
                               const QString &caption = QString(),
                               const QString &dir = QString());

    /**
     * @brief getSaveFileName - used for save current document
     * This will be a sheet dialog under OSX
     * @param parent The parent of the file dialog
     * @param caption Caption of the file dialog
     * @param dir Directory when file dialog appear
     * @param mimeList List of mimetypes user can select in the file dialog
     * @param selectedMime Mimetype selected by default
     * @return The name of the entry user selected in the file dialog
     */
    QString getSaveFileName(QWidget *parent = 0,
                            const QString &caption = QString(),
                            const QString &dir = QString(),
                            const QStringList &mimeList = QStringList(),
                            const QString &selectedMime = 0);

    /**
     * @brief getSaveFileName - used for save as or export
     * @param parent The parent of the file dialog
     * @param caption Caption of the file dialog
     * @param dir Directory when file dialog appear
     * @param mimeList List of mimetypes user can select in the file dialog
     * @param selectedMime Mimetype selected by default
     * @return The name of the entry user selected in the file dialog
     */
    QString getExportFileName(QWidget *parent = 0,
                              const QString &caption = QString(),
                              const QString &dir = QString(),
                              const QStringList &mimeList = QStringList(),
                              const QString &selectedMime = 0);

private slots:
    void getFileName(QString filename);
    void getFileNames(QStringList filenames);

private:
    const QString getFilterString(const QStringList &mimeList);
    const QString getFilterString(const QString &selectedMime);
    QFileDialog *initDialog(QWidget *parent,
                            const QString &caption,
                            const QString &dir,
                            const QStringList &mimeList,
                            const QString &selectedMime,
                            QFileDialog::AcceptMode aMode,
                            QFileDialog::FileMode fMode);

    QString m_fileName;
    QStringList m_fileNames;
};

#endif /* KOFILEDIALOG_H */
