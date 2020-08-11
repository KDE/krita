/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
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

#ifndef DLG_EXPORTSTORYBOARD
#define DLG_EXPORTSTORYBOARD

#include <QWidget>
#include <KoDialog.h>
#include "ui_wdgexportstoryboard.h"

enum ExportFormat : unsigned int
{
    PDF,
    SVG
};

enum PageSize : int
{
    None = -1,
    A3,
    A4
};

class WdgExportStoryboard : public QWidget, public Ui::WdgExportStoryboard
{
    Q_OBJECT

public:

    WdgExportStoryboard(QWidget* parent)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

class DlgExportStoryboard: public KoDialog
{
    Q_OBJECT
public:
    DlgExportStoryboard(ExportFormat format);
    ~DlgExportStoryboard() override;
    int firstItem() const;
    int lastItem() const;
    int rows() const;
    int columns() const;
    PageSize pageSize() const;
    QString exportSvgFile() const;
    QString saveFileName() const;
    ExportFormat format() const;

private Q_SLOTS:
    void slotExportClicked();
    void slotSpecifySvgClicked();

private:
    WdgExportStoryboard *m_page {0};
    QString m_exportFileName;
    ExportFormat m_format;
};

#endif
