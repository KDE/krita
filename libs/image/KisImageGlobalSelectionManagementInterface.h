/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISIMAGEGLOBALSELECTIONMANAGEMENTINTERFACE_H
#define KISIMAGEGLOBALSELECTIONMANAGEMENTINTERFACE_H

#include <kis_types.h>

class KisImageGlobalSelectionManagementInterface
{
public:
    KisImageGlobalSelectionManagementInterface(KisImage *image);

    KisSelectionMaskSP deselectedGlobalSelection() const;
    void setDeselectedGlobalSelection(KisSelectionMaskSP selectionMask);

private:
    KisImage *q;
};

#endif // KISIMAGEGLOBALSELECTIONMANAGEMENTINTERFACE_H
