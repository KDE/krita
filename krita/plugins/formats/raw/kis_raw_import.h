/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_RAW_IMPORT_H_
#define KIS_RAW_IMPORT_H_

#include <KoFilter.h>

#include "ui_wdgrawimport.h"

class K3Process;
class KDialog;
class WdgRawImport;
class KoColorProfile;
class QProgressDialog;

namespace KDcrawIface
{
class RawDecodingSettings;
}

class KisRawImport : public KoFilter
{
    Q_OBJECT

public:
    KisRawImport(QObject* parent, const QStringList&);
    virtual ~KisRawImport();

public:
    virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);


private slots:

    void slotUpdatePreview();
private:
    KDcrawIface::RawDecodingSettings rawDecodingSettings();
private:
    Ui::WdgRawImport m_rawWidget;
    KDialog* m_dialog;
};

#endif // KIS_RAW_IMPORT_H_

