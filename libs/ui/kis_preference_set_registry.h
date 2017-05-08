/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_PREFERENCE_SET_REGISTRY_H_
#define KIS_PREFERENCE_SET_REGISTRY_H_

#include <QObject>
#include <QWidget>
#include <QString>

#include "KoGenericRegistry.h"

#include "kis_types.h"

#include <kritaui_export.h>

class QIcon;

/**
 * Implement this abstract class to provide a preferences pane for the preferences dialog.
 */
class KRITAUI_EXPORT KisPreferenceSet : public QWidget {
Q_OBJECT
public:
    KisPreferenceSet(QWidget* parent=0) : QWidget(parent)
    {}
    virtual QString id() = 0;
    virtual QString name() = 0;
    virtual QString header() = 0;
    virtual QIcon icon() = 0;
public Q_SLOTS:
    virtual void savePreferences() const = 0;
    virtual void loadPreferences() = 0;
    virtual void loadDefaultPreferences() = 0;
};

class KRITAUI_EXPORT KisAbstractPreferenceSetFactory {
public:
    virtual ~KisAbstractPreferenceSetFactory() {}
    virtual KisPreferenceSet* createPreferenceSet() = 0;
    virtual QString id() const = 0;
};

/**
 * This registry does not load the plugins itself: plugins with preferences panes should
 * add those panes when they are loaded themselves.
 */
class KRITAUI_EXPORT KisPreferenceSetRegistry : public QObject, public KoGenericRegistry<KisAbstractPreferenceSetFactory*>
{
public:
    KisPreferenceSetRegistry();
    ~KisPreferenceSetRegistry() override;
    static KisPreferenceSetRegistry * instance();

private:
    Q_DISABLE_COPY(KisPreferenceSetRegistry)
};

#endif // KIS_PREFERENCE_SETSPACE_REGISTRY_H_
