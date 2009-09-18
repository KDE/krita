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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_FILTER_HANDLER_H_
#define _KIS_FILTER_HANDLER_H_

#include <QObject>

#include <kis_types.h>

class KisFilterConfiguration;
class KisFilterManager;
class KisLayer;
class KisView2;
class QRect;

/**
 * XXX: this class is way too confusing.
 */
class KisFilterHandler : public QObject
{

    Q_OBJECT

public:

    KisFilterHandler(KisFilterManager* parent, KisFilterSP f, KisView2* view);
    ~KisFilterHandler();

    const KisFilterSP filter() const;

public slots:

    void showDialog();
    void reapply();
    void apply(KisNodeSP, KisFilterConfiguration*);

private slots:

    void areaDone(const QRect & rc);
    void filterDone(bool interrupted);

private:
    struct Private;
    Private* const m_d;
};

#endif
