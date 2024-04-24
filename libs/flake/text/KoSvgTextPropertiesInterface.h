/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOSVGTEXTPROPERTIESINTERFACE_H
#define KOSVGTEXTPROPERTIESINTERFACE_H

#include <QObject>
#include <KoSvgTextProperties.h>
#include <kritaflake_export.h>

/**
 * @brief The KoSvgTextPropertiesInterface class
 *
 * This is an interface that can be used by tools to communicate
 * with the KisTextPropertiesManager.
 */
class KRITAFLAKE_EXPORT KoSvgTextPropertiesInterface : public QObject
{
    Q_OBJECT
public:
    explicit KoSvgTextPropertiesInterface(QObject *parent = nullptr): QObject(parent){}

    /**
     * @brief getSelectedProperties
     * @return all KoSvgTextProperties for the given selection.
     */
    virtual QList<KoSvgTextProperties> getSelectedProperties() = 0;

    /**
     * @brief getInheritedProperties
     * The properties that should be visible when a given property
     * isn't available in common properties. This is typically the
     * paragraph properties.
     * @return what counts as the inherited properties for the given selection.
     */
    virtual KoSvgTextProperties getInheritedProperties() = 0;
    /**
     * @brief setPropertiesOnSelected
     * This sets the properties on the selection. The implementation is responsible
     * for handling the undo states.
     * @param properties -- the properties to set.
     */
    virtual void setPropertiesOnSelected(KoSvgTextProperties properties, QSet<KoSvgTextProperties::PropertyId> removeProperties = QSet<KoSvgTextProperties::PropertyId>()) = 0;
Q_SIGNALS:
    void textSelectionChanged();
};


#endif // KOSVGTEXTPROPERTIESINTERFACE_H
