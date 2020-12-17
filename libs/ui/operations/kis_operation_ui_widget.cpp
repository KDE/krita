/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "kis_operation_ui_widget.h"

class Q_DECL_HIDDEN KisOperationUIWidget::Private {

public:
    Private() {}
    QString caption;
};


KisOperationUIWidget::KisOperationUIWidget(const QString& caption, QWidget* parent)
  : QWidget(parent)
  , d(new KisOperationUIWidget::Private)
{
    d->caption = caption;
}

KisOperationUIWidget::~KisOperationUIWidget()
{
    delete d;
}

QString KisOperationUIWidget::caption() const
{
    return d->caption;
}
