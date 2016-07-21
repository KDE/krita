/*
 *
 * Copyright (c) 2016 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_opcua_server.h"

#include <QMetaProperty>
#include <QDebug>

#include "open62541.h"

QMap<QString, QPair<QObject*, int> > m_variableMap;

QString uaStringToQString(const UA_String& uaString)
{
    int length = uaString.length;
    char* cstring = new char[length+1];
    memcpy(cstring, uaString.data, length );
    cstring[length] = '\0';

    QString result = QString(cstring);

    delete[] cstring;
    return result;
}

static UA_StatusCode readVariable(void *handle, const UA_NodeId nodeid, UA_Boolean sourceTimeStamp,
                                 const UA_NumericRange *range, UA_DataValue *dataValue) {
    dataValue->hasValue = true;

    QString id = uaStringToQString(nodeid.identifier.string);

    if(!m_variableMap.contains(id)){
        return UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    QPair<QObject*, int> property = m_variableMap[id];

    QObject* object = property.first;
    QMetaProperty prop = object->metaObject()->property(property.second);

    QVariant value = prop.read(object);

    switch (value.type()) {
        case QVariant::Int:
        {
            UA_Int32 intValue = value.toInt();
            UA_Variant_setScalarCopy(&dataValue->value, &intValue, &UA_TYPES[UA_TYPES_INT32]);
            break;
        }
        case QVariant::Double:
        {
            UA_Double doubleValue = value.toDouble();
            UA_Variant_setScalarCopy(&dataValue->value, &doubleValue, &UA_TYPES[UA_TYPES_DOUBLE]);
            break;
        }
        default:
            break;
    }

    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode writeVariable(void *handle, const UA_NodeId nodeid,
                                   const UA_Variant *data, const UA_NumericRange *range) {

    QString id = uaStringToQString(nodeid.identifier.string);

    if(!m_variableMap.contains(id)){
        return UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    QPair<QObject*, int> property = m_variableMap[id];

    QObject* object = property.first;
    QMetaProperty prop = object->metaObject()->property(property.second);

    QVariant value;
    if(UA_Variant_isScalar(data) && data->data) {
        if(data->type == &UA_TYPES[UA_TYPES_INT32]) {
            int intValue = *(UA_UInt32*)data->data;
            value = QVariant(intValue);

        }
        else if(data->type == &UA_TYPES[UA_TYPES_DOUBLE]) {
            double doubleValue = *(UA_Double*)data->data;
            value = QVariant(doubleValue);
        } else {
            return UA_STATUSCODE_BADUNEXPECTEDERROR;
        }
    }

    prop.write(object, value);

    return UA_STATUSCODE_GOOD;
}

KisOpcUaServer::KisOpcUaServer()
{

}

void KisOpcUaServer::run()
{
    bool running = true;

    UA_ServerConfig config = UA_ServerConfig_standard;
    UA_ServerNetworkLayer nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, 16664);
    config.networkLayers = &nl;
    config.networkLayersSize = 1;
    UA_Server *server = UA_Server_new(config);

    int objectIndex = 1000;
    foreach(QObject* object, m_objects) {

        UA_ObjectAttributes object_attr;
        UA_ObjectAttributes_init(&object_attr);
        object_attr.description = UA_LOCALIZEDTEXT("en_US", "");
        object_attr.displayName = UA_LOCALIZEDTEXT("en_US", object->objectName().toLatin1().data());
        UA_Server_addObjectNode(server, UA_NODEID_NUMERIC(1, objectIndex),
            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), UA_QUALIFIEDNAME(1, object->objectName().toLatin1().data()),
            UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), object_attr, NULL, NULL);

        int propertyCount = object->metaObject()->propertyCount();

        for(int i = 0; i < propertyCount; i++)
        {
            QMetaProperty property = object->metaObject()->property(i);

            QString variableName = object->objectName() + '.' + QString(property.name());
            m_variableMap[variableName] = QPair<QObject*, int>(object, i);

            QString browseName = QString(property.name());

            UA_NodeId variableNodeId = UA_NODEID_STRING(1, variableName.toLatin1().data());
            UA_QualifiedName variableNodeName = UA_QUALIFIEDNAME(1, browseName.toLatin1().data());

            UA_DataSource dataSource;
            dataSource.handle = 0;
            dataSource.read = &readVariable;
            dataSource.write = &writeVariable;

            UA_VariableAttributes attr;
            UA_VariableAttributes_init(&attr);
            attr.description = UA_LOCALIZEDTEXT("en_US", variableName.toLatin1().data());
            attr.displayName = UA_LOCALIZEDTEXT("en_US", variableName.toLatin1().data());

            UA_StatusCode retval = UA_Server_addDataSourceVariableNode(server, variableNodeId,
                                                                      UA_NODEID_NUMERIC(1, objectIndex),
                                                                      UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                                                      variableNodeName, UA_NODEID_NULL, attr, dataSource, NULL);
        }
        objectIndex++;
    }

    UA_Server_run_startup(server);
//    if(retval != UA_STATUSCODE_GOOD)
//    {
//        UA_Server_delete(server);
//        nl.deleteMembers(&nl);
//        return;
//    }

    while(running) {
        UA_UInt16 timeout = UA_Server_run_iterate(server, false);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = timeout * 1000;
        select(0, NULL, NULL, NULL, &tv);
    }
    UA_Server_run_shutdown(server);
}

void KisOpcUaServer::addObject(QObject *object)
{
    m_objects.append(object);
}

