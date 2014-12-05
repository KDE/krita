/*
 *  Copyright (c) 2014 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KRANIM_SEQUENCE_H_
#define _KRANIM_SEQUENCE_H_

#include "ui_kis_wdg_options_kranimseq.h"

#include <QVariantList>
#include <KisImportExportFilter.h>

/**
 * Exports the animation as a PNG sequence.
 */
class KisWdgOptionsKranimseq : public QWidget, public Ui::KisWdgOptionsKranimseq
{
    Q_OBJECT

public:
    KisWdgOptionsKranimseq(QWidget* parent) : QWidget(parent) {
        setupUi(this);
    }

};

class KranimSequence : public KisImportExportFilter
{
    Q_OBJECT

public:
    KranimSequence(QObject* parent, const QVariantList &);
    virtual ~KranimSequence();

public:
    virtual KisImportExportFilter::ConversionStatus convert(const QByteArray &from, const QByteArray &to);

private:
    KisWdgOptionsKranimseq* m_wdg;
};

#endif
