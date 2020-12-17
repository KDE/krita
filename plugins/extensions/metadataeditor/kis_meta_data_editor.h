/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
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
    ~KisMetaDataEditor() override;
public Q_SLOTS:
    void accept() override;
private:
    Private* const d;
};

#endif
