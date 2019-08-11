/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_GUIDES_MANAGER_H
#define __KIS_GUIDES_MANAGER_H

#include <QScopedPointer>
#include <QObject>
#include "kritaui_export.h"
#include <KoUnit.h>

class KisView;
class KisActionManager;
class KisCanvasDecoration;
class KisGuidesConfig;


class KRITAUI_EXPORT KisGuidesManager : public QObject
{
    Q_OBJECT
public:
    KisGuidesManager(QObject *parent = 0);
    ~KisGuidesManager() override;

    void setup(KisActionManager *actionManager);
    void setView(QPointer<KisView> view);

    bool showGuides() const;
    bool lockGuides() const;
    bool snapToGuides() const;
    bool rulersMultiple2() const;

    KoUnit::Type unitType() const;

    bool eventFilter(QObject *obj, QEvent *event) override;

Q_SIGNALS:
    void sigRequestUpdateGuidesConfig(const KisGuidesConfig &config);

public Q_SLOTS:
    void setGuidesConfig(const KisGuidesConfig &config);
    void slotDocumentRequestedConfig(const KisGuidesConfig &config);

    void setShowGuides(bool value);
    void setLockGuides(bool value);
    void setSnapToGuides(bool value);
    void setRulersMultiple2(bool value);
    void setUnitType(KoUnit::Type type);

    void slotGuideCreationInProgress(Qt::Orientation orientation, const QPoint &globalPos);
    void slotGuideCreationFinished(Qt::Orientation orientation, const QPoint &globalPos);

    void slotShowSnapOptions();

    void setSnapOrthogonal(bool value);
    void setSnapNode(bool value);
    void setSnapExtension(bool value);
    void setSnapIntersection(bool value);
    void setSnapBoundingBox(bool value);
    void setSnapImageBounds(bool value);
    void setSnapImageCenter(bool value);
    void setSnapToPixel(bool value);

    void slotUploadConfigToDocument();

private:
    void setGuidesConfigImpl(const KisGuidesConfig &value, bool emitModified = true);
    void attachEventFilterImpl(bool value);
    void syncActionsStatus();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_GUIDES_MANAGER_H */
