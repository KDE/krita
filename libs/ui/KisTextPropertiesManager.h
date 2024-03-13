/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISTEXTPROPERTYMANAGER_H
#define KISTEXTPROPERTYMANAGER_H

#include <QObject>
#include "kritaui_export.h"

class KisView;
class KisCanvasResourceProvider;
class KoSvgTextPropertiesInterface;

/**
 * @brief The KisTextPropertyManager class
 *
 * This class handles taking text property data from the currently selected
 * text objects and then setting it on the resource provider.
 */
class KRITAUI_EXPORT KisTextPropertiesManager : public QObject
{
    Q_OBJECT
public:
    KisTextPropertiesManager(QObject *parent = nullptr);
    ~KisTextPropertiesManager();

    void setView(QPointer<KisView> imageView);
    void setCanvasResourceProvider(KisCanvasResourceProvider *provider);
    void setTextPropertiesInterface(KoSvgTextPropertiesInterface *interface);

private Q_SLOTS:
    void slotShapeSelectionChanged();
    void slotInterfaceSelectionChanged();
    void slotTextPropertiesChanged();
private:

    struct Private;
    QScopedPointer<Private> d;

};

#endif // KISTEXTPROPERTYMANAGER_H
