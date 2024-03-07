/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISTEXTPROPERTYMANAGER_H
#define KISTEXTPROPERTYMANAGER_H

#include <QObject>

class KisView;
class KisCanvasResourceProvider;

/**
 * @brief The KisTextPropertyManager class
 *
 * This class handles taking text property data from the currently selected
 * text objects and then setting it on the resource provider.
 */
class KisTextPropertiesManager : public QObject
{
    Q_OBJECT
public:
    KisTextPropertiesManager(QObject *parent = nullptr);
    ~KisTextPropertiesManager();

    void setView(QPointer<KisView> imageView);
    void setCanvasResourceProvider(KisCanvasResourceProvider *provider);

private Q_SLOTS:
    void slotShapeSelectionChanged();
private:

    struct Private;
    QScopedPointer<Private> d;

};

#endif // KISTEXTPROPERTYMANAGER_H
