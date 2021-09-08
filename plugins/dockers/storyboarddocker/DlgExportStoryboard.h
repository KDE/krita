/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef DLG_EXPORTSTORYBOARD
#define DLG_EXPORTSTORYBOARD

#include <QWidget>
#include <QPageLayout>
#include <QPageSize>

#include <KoDialog.h>
#include "ui_wdgexportstoryboard.h"

class QPageSize;
class KisTimeSpan;

enum ExportFormat
{
    PDF = 0,
    SVG
};

enum ExportLayout
{
    ROWS = 0,
    COLUMNS = 1,
    GRID = 2,
    SVG_TEMPLATE = 3
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
    int rows() const;
    int columns() const;
    QPageSize pageSize() const;
    QPageLayout::Orientation pageOrientation() const;
    bool layoutSpecifiedBySvgFile() const;
    QString layoutSvgFile() const;
    QString saveFileName() const;
    ExportFormat format() const;
    ExportLayout exportLayout() const;
    int fontSize() const;
    void setUsableMaximums(QPageSize pPageSize, QPageLayout::Orientation pOrientation, ExportLayout pLayout );

private Q_SLOTS:
    void slotExportClicked();
    void slotLayoutChanged(int state);
    void slotPageSettingsChanged(int);

private:
    WdgExportStoryboard *m_page {0};
    QString m_exportFileName;
    ExportFormat m_format;
};

#endif
