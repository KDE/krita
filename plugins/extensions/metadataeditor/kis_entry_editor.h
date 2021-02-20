/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_ENTRY_EDITOR_H_
#define _KIS_ENTRY_EDITOR_H_

#include <QWidget>

class QString;

namespace KisMetaData
{
class Store;
}

class KisEntryEditor : public QObject
{
    Q_OBJECT
    struct Private;
public:
    KisEntryEditor(QWidget *obj, KisMetaData::Store* store, QString key, QString propertyName, QString structField, int arrayIndex);
    ~KisEntryEditor() override;
public Q_SLOTS:
    void valueEdited();
    void valueChanged();
Q_SIGNALS:
    void valueHasBeenEdited();
private:
    Private* const d;
};

#endif
