/*
 * modify_selection.h -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Michael Thaler (michael.thaler@physik.tu-muenchen.de)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef MODIFY_SELECTION_H
#define MODIFY_SELECTION_H

#include <QVariant>

#include <KisActionPlugin.h>

class ModifySelection : public KisActionPlugin
{
    Q_OBJECT
public:
    ModifySelection(QObject *parent, const QVariantList &);
    ~ModifySelection() override;
};

#endif // MODIFY_SELECTION_H
