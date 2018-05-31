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

#ifndef __KIS_MULTINODE_PROPERTY_H
#define __KIS_MULTINODE_PROPERTY_H

#include <QObject>
#include <QCheckBox>
#include <QPointer>
#include <QRegExp>
#include <QBitArray>

#include <kundo2command.h>
#include <KoColorSpace.h>
#include <KoChannelInfo.h>

#include "kis_node.h"
#include "kis_layer.h"
#include "kis_layer_utils.h"

#include "kritaui_export.h"

class KisMultinodePropertyInterface;
class MultinodePropertyBaseConnector;
template <class PropertyAdapter> class MultinodePropertyBoolConnector;
template <class PropertyAdapter> class KisMultinodeProperty;

/******************************************************************/
/*               Adapters                                         */
/******************************************************************/

struct BaseAdapter {
    static KisNodeList filterNodes(KisNodeList nodes) { return nodes; }

    void setNumNodes(int numNodes) { m_numNodes = numNodes; }
    int m_numNodes = 0;
};

struct CompositeOpAdapter : public BaseAdapter {
    typedef QString ValueType;
    typedef MultinodePropertyBaseConnector ConnectorType;
    static const bool forceIgnoreByDefault = false;

    static ValueType propForNode(KisNodeSP node) {
        return node->compositeOpId();
    }

    static void setPropForNode(KisNodeSP node, const ValueType &value, int index) {
        Q_UNUSED(index);
        node->setCompositeOpId(value);
    }
};

struct NameAdapter : public BaseAdapter {
    typedef QString ValueType;
    typedef MultinodePropertyBaseConnector ConnectorType;
    static const bool forceIgnoreByDefault = true;

    ValueType propForNode(KisNodeSP node) {
        return m_numNodes == 1 ? node->name() : stripName(node->name());
    }

    void setPropForNode(KisNodeSP node, const ValueType &value, int index) {
        QString name;

        if (index < 0 || m_numNodes == 1) {
            name = value;
        } else {
            name = QString("%1 %2").arg(stripName(value)).arg(index);
        }

        node->setName(name);
    }

private:
    static QString stripName(QString name) {
        QRegExp rexp("^(.+) (\\d{1,3})$");
        int pos = rexp.indexIn(name);
        if (pos > -1) {
            name = rexp.cap(1);
        }

        return name;
    }
};

struct ColorLabelAdapter : public BaseAdapter {
    typedef int ValueType;
    typedef MultinodePropertyBaseConnector ConnectorType;
    static const bool forceIgnoreByDefault = false;

    static ValueType propForNode(KisNodeSP node) {
        return node->colorLabelIndex();
    }

    static void setPropForNode(KisNodeSP node, const ValueType &value, int index) {
        Q_UNUSED(index);
        node->setColorLabelIndex(value);
    }
};

struct OpacityAdapter : public BaseAdapter {
    typedef int ValueType;
    typedef MultinodePropertyBaseConnector ConnectorType;
    static const bool forceIgnoreByDefault = false;

    static ValueType propForNode(KisNodeSP node) {
        return qRound(node->opacity() / 255.0 * 100);
    }

    static void setPropForNode(KisNodeSP node, const ValueType &value, int index) {
        Q_UNUSED(index);
        node->setOpacity(qRound(value * 255.0 / 100));
    }
};

inline uint qHash(const KisBaseNode::Property &prop, uint seed = 0) {
    return qHash(prop.name, seed);
}

struct LayerPropertyAdapter : public BaseAdapter {
    typedef bool ValueType;
    typedef MultinodePropertyBoolConnector<LayerPropertyAdapter> ConnectorType;
    static const bool forceIgnoreByDefault = false;

    LayerPropertyAdapter(const QString &propName) : m_propName(propName) {}

    ValueType propForNode(KisNodeSP node) {
        KisBaseNode::PropertyList props = node->sectionModelProperties();
        Q_FOREACH (const KisBaseNode::Property &prop, props) {
            if (prop.name == m_propName) {
                return prop.state.toBool();
            }
        }

        return false;
    }

    void setPropForNode(KisNodeSP node, const ValueType &value, int index) {
        Q_UNUSED(index);
        bool stateChanged = false;

        KisBaseNode::PropertyList props = node->sectionModelProperties();
        KisBaseNode::PropertyList::iterator it = props.begin();
        KisBaseNode::PropertyList::iterator end = props.end();
        for (; it != end; ++it) {
            if (it->name == m_propName) {
                it->state = value;
                stateChanged = true;
                break;
            }
        }

        if (stateChanged) {
            node->setSectionModelProperties(props);
        }
    }

    QString name() const {
        return m_propName;
    }

    static KisBaseNode::PropertyList adaptersList(KisNodeList nodes) {
        QHash<QString, std::pair<KisBaseNode::Property, int>> adapters;

        Q_FOREACH (KisNodeSP node, nodes) {
            int sortingIndex = 0;
            KisBaseNode::PropertyList props = node->sectionModelProperties();
            Q_FOREACH (const KisBaseNode::Property &prop, props) {
                if (prop.state.type() != QVariant::Bool) continue;

                if (!adapters.contains(prop.id)) {
                    adapters.insert(prop.id, std::make_pair(prop, sortingIndex));
                } else {
                    adapters[prop.id].second = qMin(adapters[prop.id].second, sortingIndex);
                }
                sortingIndex++;
            }
        }

        QMultiMap<int, KisBaseNode::Property> sortedAdapters;
        auto it = adapters.constBegin();
        auto end = adapters.constEnd();
        for (; it != end; ++it) {
            KisBaseNode::Property prop;
            int sortingIndex = 0;
            std::tie(prop, sortingIndex) = it.value();

            sortedAdapters.insert(sortingIndex, prop);
        }

        return sortedAdapters.values();
    }

private:
    QString m_propName;
};

struct ChannelFlagAdapter : public BaseAdapter {
    typedef bool ValueType;
    typedef MultinodePropertyBoolConnector<ChannelFlagAdapter> ConnectorType;
    static const bool forceIgnoreByDefault = false;

    struct Property {
        Property(QString _name, int _channelIndex) : name(_name), channelIndex(_channelIndex) {}
        QString name;
        int channelIndex;
    };
    typedef QList<Property> PropertyList;

    ChannelFlagAdapter(const Property &prop) : m_prop(prop) {}

    ValueType propForNode(KisNodeSP node) {
        KisLayerSP layer = toLayer(node);
        Q_ASSERT(layer);

        QBitArray flags = layer->channelFlags();
        if (flags.isEmpty()) return true;

        return flags.testBit(m_prop.channelIndex);
    }

    void setPropForNode(KisNodeSP node, const ValueType &value, int index) {
        Q_UNUSED(index);
        KisLayerSP layer = toLayer(node);
        Q_ASSERT(layer);

        QBitArray flags = layer->channelFlags();
        if (flags.isEmpty()) {
            flags = QBitArray(layer->colorSpace()->channelCount(), true);
        }

        if (flags.testBit(m_prop.channelIndex) != value) {
            flags.setBit(m_prop.channelIndex, value);
            layer->setChannelFlags(flags);
        }
    }

    QString name() const {
        return m_prop.name;
    }

    static PropertyList adaptersList(KisNodeList nodes) {
        PropertyList props;

        {
            bool nodesDiffer = KisLayerUtils::checkNodesDiffer<const KoColorSpace*>(nodes, [](KisNodeSP node) { return node->colorSpace(); });

            if (nodesDiffer) {
                return props;
            }
        }


        QList<KoChannelInfo*> channels = nodes.first()->colorSpace()->channels();

        int index = 0;
        Q_FOREACH (KoChannelInfo *info, channels) {
            props << Property(info->name(), index);
            index++;
        }

        return props;
    }

    static KisNodeList filterNodes(KisNodeList nodes) {
        KisNodeList filteredNodes;
        Q_FOREACH (KisNodeSP node, nodes) {
            if (toLayer(node)) {
                filteredNodes << node;
            }
        }
        return filteredNodes;
    }
private:
    static KisLayerSP toLayer(KisNodeSP node) {
        return qobject_cast<KisLayer*>(node.data());
    }
private:
    Property m_prop;
};

/******************************************************************/
/*               MultinodePropertyConnectorInterface              */
/******************************************************************/

class KRITAUI_EXPORT MultinodePropertyConnectorInterface : public QObject
{
    Q_OBJECT
public:
    ~MultinodePropertyConnectorInterface() override;

    /**
     * Public interface
     */
    virtual void connectIgnoreCheckBox(QCheckBox *ignoreBox) = 0;
    void connectValueChangedSignal(const QObject *receiver, const char *method, Qt::ConnectionType type = Qt::AutoConnection);

    /**
     * Clicking on this widget will automatically enable it,
     * setting "Ignored" property to false.
     *
     * Default implementation does nothing.
     */
    virtual void connectAutoEnableWidget(QWidget *widget);

Q_SIGNALS:
    void sigValueChanged();

protected Q_SLOTS:
    virtual void slotIgnoreCheckBoxChanged(int state) = 0;

public:
    /**
     * Interface for KisMultinodeProperty's notifications
     */
    virtual void notifyValueChanged();
    virtual void notifyIgnoreChanged() = 0;
};

/******************************************************************/
/*               MultinodePropertyBaseConnector                   */
/******************************************************************/

class KRITAUI_EXPORT MultinodePropertyBaseConnector : public MultinodePropertyConnectorInterface
{
public:
    MultinodePropertyBaseConnector(KisMultinodePropertyInterface *parent);

    void connectIgnoreCheckBox(QCheckBox *ignoreBox) override;
    void notifyIgnoreChanged() override;

    void connectAutoEnableWidget(QWidget *widget) override;

protected:
    void slotIgnoreCheckBoxChanged(int state) override;

private:
    QPointer<QCheckBox> m_ignoreBox;
    KisMultinodePropertyInterface *m_parent;
};

/******************************************************************/
/*               MultinodePropertyBoolConnector                   */
/******************************************************************/

template <class PropertyAdapter>
class MultinodePropertyBoolConnector : public MultinodePropertyConnectorInterface
{
    typedef KisMultinodeProperty<PropertyAdapter> PropertyType;
public:
    MultinodePropertyBoolConnector(PropertyType *parent)
        : m_parent(parent)
    {
    }

    void connectIgnoreCheckBox(QCheckBox *ignoreBox) override {
        m_ignoreBox = ignoreBox;

        if ((!m_parent->isIgnored() && !m_parent->savedValuesDiffer())
            || m_parent->haveTheOnlyNode()) {

            m_ignoreBox->setTristate(false);
        } else {
            m_ignoreBox->setTristate(true);
        }
        connect(m_ignoreBox, SIGNAL(stateChanged(int)), SLOT(slotIgnoreCheckBoxChanged(int)));
    }

    void notifyIgnoreChanged() override {
        // noop
    }

    void notifyValueChanged() override {
        if (m_ignoreBox) {
            Qt::CheckState newState =
                m_parent->isIgnored() ? Qt::PartiallyChecked :
                bool(m_parent->value()) ? Qt::Checked :
                Qt::Unchecked;

            if (m_ignoreBox->checkState() != newState) {
                m_ignoreBox->setCheckState(newState);
            }
        }
        MultinodePropertyConnectorInterface::notifyValueChanged();
    }
protected:
    void slotIgnoreCheckBoxChanged(int state) override {
        if (state == Qt::PartiallyChecked) {
            m_parent->setIgnored(true);
        } else {
            m_parent->setIgnored(false);
            m_parent->setValue(bool(state == Qt::Checked));
        }
    }

private:
    QPointer<QCheckBox> m_ignoreBox;
    PropertyType *m_parent;
};

/******************************************************************/
/*               MultinodePropertyUndoCommand                     */
/******************************************************************/

template <class PropertyAdapter>
class MultinodePropertyUndoCommand : public KUndo2Command
{
public:
    typedef typename PropertyAdapter::ValueType ValueType;
public:
    MultinodePropertyUndoCommand(PropertyAdapter propAdapter,
                                 KisNodeList nodes,
                                 const QList<ValueType> &oldValues,
                                 ValueType newValue,
                                 KUndo2Command *parent = 0)
        : KUndo2Command(parent),
          m_propAdapter(propAdapter),
          m_nodes(nodes),
          m_oldValues(oldValues),
          m_newValue(newValue)
    {
    }

    void undo() override {
        int index = 0;
        Q_FOREACH (KisNodeSP node, m_nodes) {
            m_propAdapter.setPropForNode(node, m_oldValues[index], -1);
            index++;
        }
    }

    void redo() override {
        int index = 0;
        Q_FOREACH (KisNodeSP node, m_nodes) {
            m_propAdapter.setPropForNode(node, m_newValue, index);
            index++;
        }
    }

private:
    PropertyAdapter m_propAdapter;
    KisNodeList m_nodes;
    QList<ValueType> m_oldValues;
    ValueType m_newValue;
};

/******************************************************************/
/*               KisMultinodePropertyInterface                    */
/******************************************************************/

class KRITAUI_EXPORT KisMultinodePropertyInterface
{
public:
    KisMultinodePropertyInterface();
    virtual ~KisMultinodePropertyInterface();

    virtual void rereadCurrentValue() = 0;

    virtual void setIgnored(bool value) = 0;
    virtual bool isIgnored() const = 0;

    virtual bool savedValuesDiffer() const = 0;
    virtual bool haveTheOnlyNode() const = 0;

    virtual void connectValueChangedSignal(const QObject *receiver, const char *method, Qt::ConnectionType type = Qt::AutoConnection) = 0;
    virtual void connectIgnoreCheckBox(QCheckBox *ignoreBox) = 0;

    virtual void connectAutoEnableWidget(QWidget *widget) = 0;

    virtual KUndo2Command* createPostExecutionUndoCommand() = 0;
};

typedef QSharedPointer<KisMultinodePropertyInterface> KisMultinodePropertyInterfaceSP;

/******************************************************************/
/*               KisMultinodeProperty                             */
/******************************************************************/

template <class PropertyAdapter>
class KisMultinodeProperty : public KisMultinodePropertyInterface
{
public:
    typedef typename PropertyAdapter::ValueType ValueType;
    typedef typename PropertyAdapter::ConnectorType ConnectorType;
public:
    KisMultinodeProperty(KisNodeList nodes, PropertyAdapter adapter = PropertyAdapter())
        : m_nodes(PropertyAdapter::filterNodes(nodes)),
          m_savedValuesDiffer(false),
          m_propAdapter(adapter),
          m_connector(new ConnectorType(this))
    {
        Q_ASSERT(!m_nodes.isEmpty());
        m_propAdapter.setNumNodes(m_nodes.size());

        ValueType lastValue = m_propAdapter.propForNode(m_nodes.first());
        Q_FOREACH (KisNodeSP node, m_nodes) {
            ValueType value = m_propAdapter.propForNode(node);
            m_savedValues.append(value);

            if (value != lastValue) {
                m_savedValuesDiffer = true;
            }

            lastValue = value;
        }

        m_isIgnored =
            m_nodes.size() > 1 && PropertyAdapter::forceIgnoreByDefault ?
            true : m_savedValuesDiffer;

        m_currentValue = defaultValue();
    }
    ~KisMultinodeProperty() override {}

    void rereadCurrentValue() override {
        if (m_isIgnored) return;

        ValueType lastValue = m_propAdapter.propForNode(m_nodes.first());
        Q_FOREACH (KisNodeSP node, m_nodes) {
            ValueType value = m_propAdapter.propForNode(node);

            if (value != lastValue) {
                qWarning() << "WARNING: mutiprops: values differ after reread!";
            }

            lastValue = value;
        }

        if (lastValue != m_currentValue) {
            m_currentValue = lastValue;
            m_connector->notifyValueChanged();
        }
    }

    void setValue(const ValueType &value) {
        Q_ASSERT(!m_isIgnored);
        if (value == m_currentValue) return;

        int index = 0;

        Q_FOREACH (KisNodeSP node, m_nodes) {
            m_propAdapter.setPropForNode(node, value, index);
            index++;
        }

        m_currentValue = value;
        m_connector->notifyValueChanged();
    }

    ValueType value() const {
        return m_currentValue;
    }

    void setIgnored(bool value) override {
        if (value == m_isIgnored) return;

        m_isIgnored = value;
        if (m_isIgnored) {
            int index = 0;
            Q_FOREACH (KisNodeSP node, m_nodes) {
                m_propAdapter.setPropForNode(node, m_savedValues[index], -1);
                index++;
            }
            m_currentValue = defaultValue();
        } else {
            int index = 0;
            Q_FOREACH (KisNodeSP node, m_nodes) {
                m_propAdapter.setPropForNode(node, m_currentValue, index);
                index++;
            }
        }

        m_connector->notifyValueChanged();
        m_connector->notifyIgnoreChanged();
    }

    bool isIgnored() const override {
        return m_isIgnored;
    }

    KUndo2Command* createPostExecutionUndoCommand() override {
        KIS_ASSERT_RECOVER(!m_isIgnored) { return new KUndo2Command(); }

        return new MultinodePropertyUndoCommand<PropertyAdapter>(m_propAdapter, m_nodes,
                                                                 m_savedValues, m_currentValue);
    }

    // TODO: disconnect methods...
    void connectIgnoreCheckBox(QCheckBox *ignoreBox) override {
        m_connector->connectIgnoreCheckBox(ignoreBox);
    }

    void connectValueChangedSignal(const QObject *receiver, const char *method, Qt::ConnectionType type = Qt::AutoConnection) override {
        m_connector->connectValueChangedSignal(receiver, method, type);
    }

    void connectAutoEnableWidget(QWidget *widget) override {
        m_connector->connectAutoEnableWidget(widget);
    }

    /**
     * Interface for the connector
     */

    bool savedValuesDiffer() const override {
        return m_savedValuesDiffer;
    }

    bool haveTheOnlyNode() const override {
        return m_nodes.size() == 1;
    }

private:
    ValueType defaultValue() const {
        return m_savedValues.first();
    }

private:
    bool m_isIgnored;
    ValueType m_currentValue;

    KisNodeList m_nodes;
    QList<ValueType> m_savedValues;

    bool m_savedValuesDiffer;
    PropertyAdapter m_propAdapter;
    QScopedPointer<MultinodePropertyConnectorInterface> m_connector;
};


typedef KisMultinodeProperty<CompositeOpAdapter> KisMultinodeCompositeOpProperty;
typedef KisMultinodeProperty<OpacityAdapter> KisMultinodeOpacityProperty;
typedef KisMultinodeProperty<NameAdapter> KisMultinodeNameProperty;
typedef KisMultinodeProperty<ColorLabelAdapter> KisMultinodeColorLabelProperty;

#endif /* __KIS_MULTINODE_PROPERTY_H */
