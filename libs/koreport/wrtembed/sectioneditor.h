/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SECTIONEDITOR_H
#define SECTIONEDITOR_H

class KoReportDesigner;
class ReportSectionDetail;
#include <QtGui/QDialog>

#include <ui_sectioneditor.h>

class SectionEditor : public QDialog, public Ui::SectionEditor
{
    Q_OBJECT

public:
    explicit SectionEditor(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~SectionEditor();

public slots:
    virtual void cbReportHeader_toggled(bool yes);
    virtual void cbReportFooter_toggled(bool yes);
    virtual void cbHeadFirst_toggled(bool yes);
    virtual void cbHeadLast_toggled(bool yes);
    virtual void cbHeadEven_toggled(bool yes);
    virtual void cbHeadOdd_toggled(bool yes);
    virtual void cbFootFirst_toggled(bool yes);
    virtual void cbFootLast_toggled(bool yes);
    virtual void cbFootEven_toggled(bool yes);
    virtual void cbFootOdd_toggled(bool yes);
    virtual void init(KoReportDesigner * rd);
    virtual void cbHeadAny_toggled(bool yes);
    virtual void cbFootAny_toggled(bool yes);

    virtual void btnAdd_clicked();
    virtual void btnEdit_clicked();
    virtual void btnRemove_clicked();
    virtual void btnMoveUp_clicked();
    virtual void brnMoveDown_clicked();
protected:
    KoReportDesigner * m_reportDesigner;
    ReportSectionDetail * m_reportSectionDetail;

protected slots:
    virtual void languageChange();

};

#endif // SECTIONEDITOR_H
