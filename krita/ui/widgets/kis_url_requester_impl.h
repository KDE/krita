/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_URL_REQUESTER_IMPL_H
#define KIS_URL_REQUESTER_IMPL_H

#include "krita_export.h"


#include <QWidget>
#include <QString>
#include <kurl.h>
#include <KoFileDialog.h>



namespace Ui {
    class KisUrlRequesterImpl;
}

class KRITAUI_EXPORT KisUrlRequesterImpl : public QWidget
{
    Q_OBJECT

public:
    explicit KisUrlRequesterImpl(QWidget *parent = 0);
    ~KisUrlRequesterImpl();

    void setStartDir(const QString &path);

    KUrl url() const;
    void setUrl(const KUrl &url);

    void setMode(KoFileDialog::DialogType mode);
    KoFileDialog::DialogType mode() const;

public slots:
    void slotSelectFile();

signals:
    void textChanged(const QString &fileName);
    void urlSelected(const KUrl &url);
    
private:
    QString fileName() const;
    void setFileName(const QString &path);

private:
    Ui::KisUrlRequesterImpl *ui;
    QString m_basePath;
    KoFileDialog::DialogType m_mode;
};

#endif // KIS_URL_REQUESTER_IMPL_H
