/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <brushengine/kis_paintop_factory.h>

#include <klocalizedstring.h>
#include <KoColorSpace.h>

KisPaintOpFactory::KisPaintOpFactory(const QStringList & whiteListedCompositeOps)
    : m_whiteListedCompositeOps(whiteListedCompositeOps), m_priority(100)
    , m_visibility(AUTO)
{
}

QStringList KisPaintOpFactory::whiteListedCompositeOps() const
{
    return m_whiteListedCompositeOps;
}

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
void KisPaintOpFactory::preinitializePaintOpIfNeeded(const KisPaintOpSettingsSP settings)
{
    Q_UNUSED(settings);
}
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

QIcon KisPaintOpFactory::icon()
{
    QPixmap p = QPixmap(22, 22);
    p.fill(Qt::transparent);
    return QIcon(p);
}

QString KisPaintOpFactory::categoryStable()
{
    return i18nc("Category of brush engines", "Brush engines");
}

KisInterstrokeDataFactory *KisPaintOpFactory::createInterstrokeDataFactory(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface) const
{
    return 0;
}

void KisPaintOpFactory::setPriority(int newPriority)
{
    m_priority = newPriority;
}


int KisPaintOpFactory::priority() const
{
    return m_priority;
}



