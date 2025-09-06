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

    /**
     * @brief setCanvasResourceProvider
     * set the canvas resource provider.
     * @param provider
     */
    void setCanvasResourceProvider(KisCanvasResourceProvider *provider);
    /**
     * @brief setTextPropertiesInterface
     * set the text properties interface. This should be done on tool activation.
     * On tool deactivation this should be set to a nullptr, so that signals
     * from the text properties manager don't get sent to the deactivated tool if
     * a tool that does not have a text properties interface is currently active.
     * @param interface -- the tool's text property interface.
     */
    void setTextPropertiesInterface(KoSvgTextPropertiesInterface *interface);

private Q_SLOTS:
    void slotInterfaceSelectionChanged();
    void slotCharacterInterfaceSelectionChanged();
    void slotTextPropertiesChanged();
    void slotCharacterPropertiesChanged();
private:

    struct Private;
    QScopedPointer<Private> d;

};

#endif // KISTEXTPROPERTYMANAGER_H
