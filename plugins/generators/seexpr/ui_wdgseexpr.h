/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
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

#include <klocalizedstring.h>

#ifndef UI_WDGSEEXPR_H
#define UI_WDGSEEXPR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <SeExpr2/UI/ExprControlCollection.h>
#include <SeExpr2/UI/ExprEditor.h>

QT_BEGIN_NAMESPACE

class Ui_WdgSeExpr
{
public:
    QVBoxLayout *rightLayout;
    ExprEditor *txtEditor;
    ExprControlCollection *wdgControls;
    QScrollArea *scrollArea;

    void setupUi(QWidget *WdgSeExpr)
    {
        if (WdgSeExpr->objectName().isEmpty())
            WdgSeExpr->setObjectName(QString::fromUtf8("WdgSeExpr"));
        WdgSeExpr->resize(500, 480);
        WdgSeExpr->setMinimumSize(500, 0);
        rightLayout = new QVBoxLayout(WdgSeExpr);
        rightLayout->setContentsMargins(0, 0, 0, 0);
        rightLayout->setObjectName(QString::fromUtf8("rightLayout"));
        wdgControls = new ExprControlCollection(WdgSeExpr);
        wdgControls->setObjectName(QString::fromUtf8("wdgControls"));
        wdgControls->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
        txtEditor = new ExprEditor(WdgSeExpr, wdgControls);
        txtEditor->setObjectName(QString::fromUtf8("txtEditor"));
        scrollArea = new QScrollArea(WdgSeExpr);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setWidgetResizable(true);
        scrollArea->setWidget(wdgControls);

        rightLayout->addWidget(scrollArea, 5);
        rightLayout->addWidget(txtEditor, 4);


        retranslateUi(WdgSeExpr);

        QMetaObject::connectSlotsByName(WdgSeExpr);
    } // setupUi

    void retranslateUi(QWidget *WdgSeExpr)
    {
        Q_UNUSED(WdgSeExpr);
    } // retranslateUi

};

namespace Ui {
    class WdgSeExpr: public Ui_WdgSeExpr {};
} // namespace Ui

QT_END_NAMESPACE

#endif // WDGSEEXPR_H

