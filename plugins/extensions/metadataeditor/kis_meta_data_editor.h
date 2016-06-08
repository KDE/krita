/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
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

#ifndef _KIS_META_DATA_EDITOR_H_
#define _KIS_META_DATA_EDITOR_H_

#include <QWidget>

#include <kpagedialog.h>

#include "ui_dublincore.h"
#include "ui_exif.h"

namespace KisMetaData
{
class Store;
}

class WdgDublinCore : public QWidget, public Ui::DublinCore
{
    Q_OBJECT

public:
    WdgDublinCore(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class WdgExif : public QWidget, public Ui::Exif
{
    Q_OBJECT

public:
    WdgExif(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};


class KisMetaDataEditor : public KPageDialog
{
    Q_OBJECT
private:
    struct Private;
public:
    KisMetaDataEditor(QWidget* parent, KisMetaData::Store* store);
    ~KisMetaDataEditor();
public Q_SLOTS:
    virtual void accept();
private:
    Private* const d;
};

#endif
