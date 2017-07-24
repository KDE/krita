%MappedType QMap<QString, Resource*>
{
%TypeHeaderCode
#include <QMap>
#include <QString>
#include <Resource.h>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMap<QString, Resource*>::const_iterator i = sipCpp->constBegin();

    while (i != sipCpp->constEnd())
    {
        QString *t1 = new QString(i.key());

        PyObject *t1obj = sipConvertFromNewType(t1, sipType_QString, sipTransferObj);
        PyObject* t2obj = sipConvertFromType(i.value(), sipType_Resource, sipTransferObj);

        if (t1obj == NULL || t2obj == NULL || PyDict_SetItem(d, t1obj, t2obj) < 0)
        {
            Py_DECREF(d);

            if (t1obj) {
              Py_DECREF(t1obj);
            }

            if (t2obj) {
              Py_DECREF(t2obj);
            }

            return NULL;
        }

        Py_DECREF(t1obj);
        Py_DECREF(t2obj);

        ++i;
    }

    return d;
%End

%ConvertToTypeCode
    PyObject *t1obj, *t2obj;
    Py_ssize_t i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
            if (!sipCanConvertToType(t1obj, sipType_QString, SIP_NOT_NONE))
                return 0;
        }

        return 1;
    }

    QMap<QString, Resource*> *qm = new QMap<QString, Resource*>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;

        QString *t1 = reinterpret_cast<QString *>(sipConvertToType(t1obj, sipType_QString, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
        Resource *t2 = reinterpret_cast<Resource*>(sipConvertToType(t2obj, sipType_Resource, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
            sipReleaseType(t1, sipType_QString, state);
            delete qm;
            return 0;
        }

        qm->insert(*t1, t2);

        sipReleaseType(t1, sipType_QString, state);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};


