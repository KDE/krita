/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_FILTER_DIALOG_H_
#define _KIS_FILTER_DIALOG_H_

#include <QDialog>

#include <kis_types.h>

class KisFilter;
class KisFilterConfiguration;

class KisFilterDialog : public QDialog {
    Q_OBJECT
    struct Private;
    public:
        KisFilterDialog(QWidget* parent, KisLayerSP device);
        ~KisFilterDialog();
        void setFilter(KisFilterSP f);
    public slots:
        void updatePreview();
    protected slots:
        void apply();
    signals:
        void sigPleaseApplyFilter(KisLayerSP, KisFilterConfiguration*);
    private:
        KisFilterDialog::Private* const d;
};

#endif
