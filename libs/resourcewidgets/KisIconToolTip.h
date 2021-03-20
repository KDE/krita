/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 1999 Carsten Pfeiffer (pfeiffer@kde.org)
 * SPDX-FileCopyrightText: 2002 Igor Jansen (rm@kde.org)
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KISICONTOOLTIP_H
#define KISICONTOOLTIP_H

#include "KoItemToolTip.h"

#include "kritaresourcewidgets_export.h"

class KoCheckerBoardPainter;

class KRITARESOURCEWIDGETS_EXPORT KisIconToolTip: public KoItemToolTip
{

public:
    KisIconToolTip();
    ~KisIconToolTip() override;

    void setFixedToolTipThumbnailSize(const QSize &size);
    void setToolTipShouldRenderCheckers(bool value);

protected:
    QTextDocument *createDocument( const QModelIndex &index ) override;

private:
    QSize m_fixedToolTipThumbnailSize;
    QScopedPointer<KoCheckerBoardPainter> m_checkersPainter;
};

#endif // KOICONTOOLTIP_H
