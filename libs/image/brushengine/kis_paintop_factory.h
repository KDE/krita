/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_PAINTOP_FACTORY_H_
#define KIS_PAINTOP_FACTORY_H_

#include "kis_types.h"
#include "kritaimage_export.h"
#include <QObject>
#include <QString>
#include <QIcon>
#include <QStringList>
#include <kis_threaded_text_rendering_workaround.h>

class KisPainter;
class KisPaintOp;
class QWidget;
class KisPaintOpConfigWidget;

/**
 * The paintop factory is responsible for creating paintops of the specified class.
 * If there is an optionWidget, the derived paintop itself must support settings,
 * and it's up to the factory to do that.
 */
class KRITAIMAGE_EXPORT KisPaintOpFactory : public QObject
{
    Q_OBJECT

public:

    enum PaintopVisibility {
        AUTO,
        ALWAYS,
        NEVER
    };

    /**
     * @param whiteListedCompositeOps list of compositeops that don't work with this paintop
     */
    KisPaintOpFactory(const QStringList & whiteListedCompositeOps = QStringList());
    ~KisPaintOpFactory() override {}

    static QString categoryStable();

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
    virtual void preinitializePaintOpIfNeeded(const KisPaintOpSettingsSP settings);
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

    /**
     * Create a KisPaintOp with the given settings and painter.
     * @param settings the settings associated with the input device
     * @param painter the painter used to draw
     * @param node the node used to draw
     * @param image the image used to draw
     */
    virtual KisPaintOp * createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image) = 0;
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString category() const = 0;

    /**
     * List of usually hidden compositeops that are useful for this paintop.
     */
    QStringList whiteListedCompositeOps() const;

    /**
     * @brief icon
     * @return the icon to represent this paintop.
     */
    virtual QIcon icon();

    /**
     * Create and return an settings object for this paintop.
     */
    virtual KisPaintOpSettingsSP createSettings() = 0;

    /**
     * create a widget that can display paintop settings
     */
    virtual KisPaintOpConfigWidget* createConfigWidget(QWidget* parent) = 0;

    /**
     * Set the priority of this paintop, as it is shown in the UI; lower number means
     * it will be show more to the front of the list.
     * @param newPriority the priority
     */
    void setPriority(int newPriority);

    int priority() const;

    /**
     * This method will be called by the registry after all paintops are loaded
     * Overwrite to let the factory do something.
     */
    virtual void processAfterLoading() {}

private:
    QStringList m_whiteListedCompositeOps;
    int m_priority;
    PaintopVisibility m_visibility;
};

#endif
