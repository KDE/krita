/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KOPRINTINGDIALOG_H
#define KOPRINTINGDIALOG_H

#include "koguiutils_export.h"

#include <QDialog>
#include <QList>

class KoShapeManager;
class KoShape;


class KOGUIUTILS_EXPORT KoPrintingDialog : public QDialog {
    Q_OBJECT
public:
    KoPrintingDialog(QWidget *parent);
    virtual ~KoPrintingDialog();

    void setShapeManager(KoShapeManager *sm);
    void setPageRange(const QList<int> &pages);

    QPrinter &printer();

protected:
    virtual void preparePage(int pageNumber) = 0;
    virtual QList<KoShape*> shapesOnPage(int pageNumber) = 0;

    KoShapeManager *shapeManager() const;
    QPainter &painter() const;

    bool isCancelled() const;

    /// reimplemented
    virtual void showEvent(QShowEvent *event);

    virtual void printingDone() { }

private:
    class Private;
    Private * const d;
    Q_PRIVATE_SLOT(d, void preparePage(const QVariant &page))
    Q_PRIVATE_SLOT(d, void printPage(const QVariant &page))
    Q_PRIVATE_SLOT(d, void cancelPressed())
};

#endif

