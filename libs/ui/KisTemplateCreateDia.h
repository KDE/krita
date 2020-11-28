/*
   This file is part of the KDE project
   SPDX-FileCopyrightText: 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   SPDX-FileCopyrightText: 2000 Werner Trobin <trobin@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIS_TEMPLATE_CREATE_DIA_H
#define KIS_TEMPLATE_CREATE_DIA_H

#include <KoDialog.h>
#include "kritaui_export.h"

class QString;
class QPixmap;
class KisDocument;
class KisTemplateCreateDiaPrivate;

/****************************************************************************
 *
 * Class: koTemplateCreateDia
 *
 ****************************************************************************/

class KRITAUI_EXPORT KisTemplateCreateDia : public KoDialog
{
    Q_OBJECT

private:
    KisTemplateCreateDia(const QString &templatesResourcePath,
                         const QString &filePath, const QPixmap &thumbnail, QWidget *parent=0 );
    ~KisTemplateCreateDia() override;

public:
    static void createTemplate(const QString &templatesResourcePath, const char *suffix,
                               KisDocument *document, QWidget *parent = 0);

private Q_SLOTS:
    void slotOk();

    void slotDefault();
    void slotCustom();
    void slotSelect();
    void slotNameChanged(const QString &name);

    void slotAddGroup();
    void slotRemove();
    void slotSelectionChanged();
private:
    void updatePixmap();
    void fillGroupTree();

private:
    KisTemplateCreateDiaPrivate * const d;
};

#endif
