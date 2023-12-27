/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KISUNIQUECOLORSET_H
#define KISUNIQUECOLORSET_H


#include "kritapigment_export.h"

#include "KoColor.h"

#include <QObject>
#include <QScopedPointer>

class KRITAPIGMENT_EXPORT KisUniqueColorSet : public QObject
{
    Q_OBJECT
public:
    explicit KisUniqueColorSet(QObject *parent = nullptr);
    ~KisUniqueColorSet() override;

    void addColor(const KoColor &color);
    KoColor color(int index) const;
    int size() const;

public Q_SLOTS:
    void clear();
Q_SIGNALS:
    void sigReset();
    void sigColorAdded(int position);
    void sigColorMoved(int from, int to);
    void sigColorRemoved(int position);
private:
    struct ColorEntry;
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KISUNIQUECOLORSET_H
