/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SVGTEXTSHAPEMANAGERBLOCKER_H
#define SVGTEXTSHAPEMANAGERBLOCKER_H

#include <KisAdaptedLock.h>
#include <KoShapeManager.h>

class SvgTextShapeManagerBlockerAdapter
{
public:
    SvgTextShapeManagerBlockerAdapter(KoShapeManager *shapeManager);

    ~SvgTextShapeManagerBlockerAdapter() = default;

    void lock();
    void unlock();

private:
    KoShapeManager *m_manager;
    bool m_managerState {false};
};

KIS_DECLARE_ADAPTED_LOCK(SvgTextShapeManagerBlocker, SvgTextShapeManagerBlockerAdapter)

#endif // SVGTEXTSHAPEMANAGERBLOCKER_H
