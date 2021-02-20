/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
