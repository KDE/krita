# -*- coding: utf-8 -*-
"""
SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>

SPDX-License-Identifier: LGPL-2.0-or-later
"""

"""
Mini Kross - a scripting solution inspired by Kross (http://kross.dipe.org/)

Technically this is one of the most important modules in Scripter.
Via the Qt meta object system it provides access to unwrapped objects.
This code uses a lot of metaprogramming magic. To fully understand it,
you have to know about metaclasses in Python
"""

import sys
try:
    from PyQt5 import sip  # Private sip module, used by modern PyQt5
except ImportError:
    import sip             # For older PyQt5 versions
from PyQt5.QtCore import QVariant, QMetaObject, Q_RETURN_ARG, Q_ARG, QObject, Qt, QMetaMethod, pyqtSignal
from PyQt5.QtGui import QBrush, QFont, QImage, QPalette, QPixmap
from PyQt5.QtWidgets import qApp


variant_converter = {
    "QVariantList": lambda v: v.toList(v),
    "QVariantMap": lambda v: toPyObject(v),
    "QPoint": lambda v: v.toPoint(),
    "str": lambda v: v.toString(),
    "int": lambda v: v.toInt()[0],
    "double": lambda v: v.toDouble()[0],
    "char": lambda v: v.toChar(),
    "QByteArray": lambda v: v.toByteArray(),
    "QPoint": lambda v: v.toPoint(),
    "QPointF": lambda v: v.toPointF(),
    "QSize": lambda v: v.toSize(),
    "QLine": lambda v: v.toLine(),
    "QStringList": lambda v: v.toStringList(),
    "QTime": lambda v: v.toTime(),
    "QDateTime": lambda v: v.toDateTime(),
    "QDate": lambda v: v.toDate(),
    "QLocale": lambda v: v.toLocale(),
    "QUrl": lambda v: v.toUrl(),
    "QRect": lambda v: v.toRect(),
    "QBrush": lambda v: QBrush(v),
    "QFont": lambda v: QFont(v),
    "QPalette": lambda v: QPalette(v),
    "QPixmap": lambda v: QPixmap(v),
    "QImage": lambda v: QImage(v),
    "bool": lambda v: v.toBool(),
    "QObject*": lambda v: wrap_variant_object(v),
    "QWidget*": lambda v: wrap_variant_object(v),
    "ActionMap": lambda v: int(v.count())
}


def wrap_variant_object(variant):
    """
    convert a QObject or a QWidget to its wrapped superclass
    """
    o = Krita.fromVariant(variant)
    return wrap(o, True)


def from_variant(variant):
    """
    convert a QVariant to a Python value
    """
    # Check whether it's really a QVariant
    if hasattr(variant, '__type__') and not (variant is None or variant.type() is None):
        typeName = variant.typeName()
        convert = variant_converter.get(typeName)
        if not convert:
            raise ValueError("Could not convert value to %s" % typeName)
        else:
            v = convert(variant)
            return v

    # Give up and return
    return variant


def convert_value(value):
    """
    Convert a given value, upcasting to the highest QObject-based class if possible,
    unpacking lists and dicts.
    """

    # Check whether it's a dict: if so, convert the keys/values
    if hasattr(value, '__class__') and issubclass(value.__class__, dict) and len(value) > 0:
        return {convert_value(k): convert_value(v) for k, v in value.items()}

    # Check whether it's a list: if so, convert the values
    if hasattr(value, '__class__') and issubclass(value.__class__, list) and len(value) > 0:
        return [convert_value(v) for v in value]

    if isinstance(value, str):
        # prefer Python strings
        return str(value)

    elif isinstance(value, PyQtClass):
        # already wrapped
        return value

    # Check whether it's a QObject
    if hasattr(value, '__class__') and issubclass(value.__class__, QObject):
        return wrap(value, True)

    if hasattr(value, '__type__') and not (value is None or value.type() is None):
        return from_variant(value)

    return value

qtclasses = {}


def wrap(obj, force=False):
    """
    If a class is not known by PyQt it will be automatically
    casted to a known wrapped super class.

    But that limits access to methods and propperties of this super class.
    So instead this functions returns a wrapper class (PyQtClass)
    which queries the metaObject and provides access to
    all slots and all properties.
    """
    if isinstance(obj, str):
        # prefer Python strings
        return str(obj)
    elif isinstance(obj, PyQtClass):
        # already wrapped
        return obj
    elif obj and isinstance(obj, QObject):
        if force or obj.__class__.__name__ != obj.metaObject().className():
            # Ah this is an unwrapped class
            obj = create_pyqt_object(obj)
    return obj


def unwrap(obj):
    """
    if wrapped returns the wrapped object
    """
    if hasattr(obj, "qt"):
        obj = obj.qt
    return obj


def is_qobject(obj):
    """
    checks if class or wrapped class is a subclass of QObject
    """
    if hasattr(obj, "__bases__") and issubclass(unwrap(obj), QObject):
        return True
    else:
        return False


def is_scripter_child(qobj):
    """
    walk up the object tree until Scripter or the root is found
    """
    found = False
    p = qobj.parent()
    while p and not found:
        if str(p.objectName()) == "Krita":
            found = True
            break
        else:
            p = p.parent()
    return found


class Error(Exception):

    """
    Base error classed. Catch this to handle exceptions coming from C++
    """


class PyQtClass(object):

    """
    Base class
    """

    def __init__(self, instance):
        self._instance = instance

    def __del__(self):
        """
        If this object is deleted it should also delete the wrapped object
        if it was created explicitly for this use.
        """
        qobj = self._instance
        if is_scripter_child(qobj):
            if len(qobj.children()):
                print("Cannot delete", qobj, "because it has child objects")
            sip.delete(qobj)

    def setProperty(self, name, value):
        self._instance.setProperty(name, value)

    def getProperty(self, name):
        return wrap(self._instance.property(name))

    def propertyNames(self):
        return list(self.__class__.__properties__.keys())

    def dynamicPropertyNames(self):
        return self._instance.dynamicPropertyNames()

    def metaObject(self):
        return self._instance.metaObject()

    def connect(self, signal, slot):
        getattr(self._instance, signal).connect(slot)

    def disconnect(self, signal, slot):
        getattr(self._instance, signal).disconnect(slot)

    def parent(self):
        return wrap(self._instance.parent())

    def children(self):
        return [wrap(c) for c in self._instance.children()]

    @property
    def qt(self):
        return self._instance

    def __getitem__(self, key):
        if isinstance(key, int):
            length = getattr(self, "length", None)
            if length is not None:
                # array protocol
                try:
                    return getattr(self, str(key))
                except AttributeError as e:
                    raise IndexError(key)
            else:
                return self.children()[key]
        else:
            return getattr(self, key)

    def __getattr__(self, name):
        # Make named child objects available as attributes like QtQml
        # Check whether the object is in the QObject hierarchy
        for child in self._instance.children():
            if str(child.objectName()) == name:
                obj = wrap(child)
                # Save found object for faster lookup
                setattr(self, name, obj)
                return obj

        # Check whether it's a property
        v = self._instance.property(name)
        return convert_value(v)

    @property
    def __members__(self):
        """
        This method is for introspection.
        Using dir(thispyqtclass_object) returns a list of
        all children, methods, properties and dynamic properties.
        """
        names = list(self.__dict__.keys())
        for c in self._instance.children():
            child_name = str(c.objectName())
            if child_name:
                names.append(child_name)
        for pn in self._instance.dynamicPropertyNames():
            names.append(str(pn))
        return names

    def __enter__(self):
        print("__enter__", self)

    def __exit__(self, exc_type, exc_value, traceback):
        print("__exit__", self, exc_type, exc_value, traceback)


class PyQtProperty(object):

    # slots for more speed
    __slots__ = ["meta_property", "name", "__doc__", "read_only"]

    def __init__(self, meta_property):
        self.meta_property = meta_property
        self.name = meta_property.name()
        self.read_only = not meta_property.isWritable()
        self.__doc__ = "%s is a %s%s" % (
            self.name, meta_property.typeName(),
            self.read_only and "  (read-only)" or ""
        )

    def get(self, obj):
        return convert_value(self.meta_property.read(obj._instance))

    def set(self, obj, value):
        self.meta_property.write(obj._instance, value)


class PyQtMethod(object):

    __slots__ = ["meta_method", "name", "args", "returnType", "__doc__"]

    def __init__(self, meta_method):
        self.meta_method = meta_method
        self.name, args = str(meta_method.methodSignature(), encoding="utf-8").split("(", 1)
        self.args = args[:-1].split(",")
        self.returnType = str(meta_method.typeName())

        types = [str(t, encoding="utf-8") for t in meta_method.parameterTypes()]
        names = [str(n, encoding="utf-8") or "arg%i" % (i + 1)
                 for i, n in enumerate(meta_method.parameterNames())]
        params = ", ".join("%s %s" % (t, n) for n, t in zip(types, names))

        self.__doc__ = "%s(%s)%s" % (
            self.name, params,
            self.returnType and (" -> %s" % self.returnType) or ""
        )

    def instancemethod(self):
        def wrapper(obj, *args):
            qargs = [Q_ARG(t, v) for t, v in zip(self.args, args)]
            invoke_args = [obj._instance, self.name]
            invoke_args.append(Qt.DirectConnection)
            rtype = self.returnType
            if rtype:
                invoke_args.append(Q_RETURN_ARG(rtype))
            invoke_args.extend(qargs)
            try:
                result = QMetaObject.invokeMethod(*invoke_args)
            except RuntimeError as e:
                raise TypeError(
                    "%s.%s(%r) call failed: %s" % (obj, self.name, args, e))
            return wrap(result)
        wrapper.__doc__ = self.__doc__
        return wrapper


# Cache on-the-fly-created classes for better speed
pyqt_classes = {}


def create_pyqt_class(metaobject):
    class_name = str(metaobject.className())
    cls = pyqt_classes.get(class_name)
    if cls:
        return cls
    attrs = {}

    properties = attrs["__properties__"] = {}
    for i in range(metaobject.propertyCount()):
        prop = PyQtProperty(metaobject.property(i))
        prop_name = str(prop.name)
        if prop.read_only:
            properties[prop_name] = attrs[prop_name] = property(prop.get, doc=prop.__doc__)
        else:
            properties[prop_name] = attrs[prop_name] = property(
                prop.get, prop.set, doc=prop.__doc__)

    methods = attrs["__methods__"] = {}
    signals = attrs["__signals__"] = {}
    for i in range(metaobject.methodCount()):
        meta_method = metaobject.method(i)
        if meta_method.methodType() != QMetaMethod.Signal:
            method = PyQtMethod(meta_method)
            method_name = method.name
            if method_name in attrs:
                # There is already a property with this name
                # So append an underscore
                method_name += "_"
            instance_method = method.instancemethod()
            instance_method.__doc__ = method.__doc__
            methods[method_name] = attrs[method_name] = instance_method
        else:
            method_name = meta_method.name()
            signal_attrs = []
            properties[bytes(method_name).decode('ascii')] = pyqtSignal(meta_method.parameterTypes())

    # Dynamically create a class with a base class and a dictionary
    cls = type(class_name, (PyQtClass,), attrs)
    pyqt_classes[class_name] = cls
    return cls


def create_pyqt_object(obj):
    """
     Wrap a QObject and make all slots and properties dynamically available.
     @type obj:  QObject
     @param obj: an unwrapped QObject
     @rtype:     PyQtClass object
     @return:    dynamically created object with all available properties and slots

     This is probably the only function you need from this module.
     Everything else are helper functions and classes.
    """
    cls = create_pyqt_class(obj.metaObject())
    return cls(obj)
