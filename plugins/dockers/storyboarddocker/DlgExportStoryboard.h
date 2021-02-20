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
enum ExportFormat : unsigned int
{
    PDF,
    SVG
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
    DlgExportStoryboard(ExportFormat format, KisTimeSpan span);
    ~DlgExportStoryboard() override;
    int firstItem() const;
    int lastItem() const;
    int rows() const;
    int columns() const;
    QPageSize pageSize() const;
    QPageLayout::Orientation pageOrientation() const;
    bool layoutSpecifiedBySvgFile() const;
    QString layoutSvgFile() const;
    QString saveFileName() const;
    QString svgFileBaseName() const;
    ExportFormat format() const;
    int fontSize() const;

private Q_SLOTS:
    void slotExportClicked();
    void slotChkUseSvgLayoutChanged(int state);

private:
    WdgExportStoryboard *m_page {0};
    QString m_exportFileName;
    ExportFormat m_format;
};

#endif
