# -*- coding: utf-8 -*-
"""
Mini Kross - a scripting solution inspired by Kross (http://kross.dipe.org/)
Technically this is one of the most important modules in Scripter.
Via the Qt meta object system it provides access to unwrapped objects.
This code uses a lot of metaprogramming magic. To fully understand it,
you have to know about metaclasses in Python
"""
from __future__ import (print_function, with_statement)

import sip
from PyQt5.QtCore import (
    QMetaObject, Q_RETURN_ARG, Q_ARG,
    QObject, Qt, QMetaMethod, pyqtSignal)
from PyQt5.QtGui import QBrush, QFont, QImage, QPalette, QPixmap
from PyQt5.QtWidgets import qApp

import sys # TODO: remove this: only for outputting debug

variant_converter = {
  "QVariantList": lambda v: from_variantlist(v),
  "QList<QVariant>": lambda v: v.toList(),
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
  "QObject*": lambda v: Krita.fromVariant(v),
  "QWidget*": lambda v: Krita.fromVariant(v),
  "ActionMap": lambda v: int(v.count())
}

def from_variant(variant):
    """
    convert a QVariant to a Python value
    """
    typeName = variant.typeName()
    convert = variant_converter.get(typeName)
    if not convert:
        raise ValueError("Could not convert value to %s" % typeName)
    else:
        return convert(variant)

def from_variantlist(variantlist):
    """
    convert QList<QVariant> to a normal Python list
    """
    return [from_variant(variant) for variant in variantlist.toList()]

def classname(obj):
    """
    return real class name
    Unwrapped classes will be represended in PyQt by a known base class.
    So obj.__class__.__name__ will not return the desired class name
    """
    return obj.metaObject().className()

qtclasses = {}

def supercast(obj):
    """
    cast a QObject subclass to the best known wrapped super class
    """
    if not qtclasses:
        # To get really all Qt classes I would have to
        # import QtNetwork, QtSvg and QtQml, too.
        import PyQt5
        qtclasses.update(
            dict([(key, value) \
                for key, value in list(PyQt5.QtCore.__dict__.items()) + list(PyQt5.QtGui.__dict__.items()) \
                if hasattr(value, "__subclasses__") and issubclass(value, QObject)])
        )
    try:
        if not issubclass(value, QObject):
            return obj
    except TypeError:
        # no class - no cast...
        return obj
    mo = obj.metaObject()
    while mo:
        cls = qtclasses.get(str(mo.className()))
        if cls:
            return sip.cast(obj, cls)
        mo = mo.superClass()
    # This should never be reached
    return obj



def wrap(obj, force=False):
    """
    If a class is not known by PyQt it will be automatically
    casted to a known wrapped super class.
    But that limits access to methods and propperties of this super class.
    So instead this functions returns a wrapper class (PyQtClass)
    which queries the metaObject and provides access to
    all slots and all properties.
    """
    #print("wrap", obj, force)
    if isinstance(obj, str):
        #print("String")
        # prefer Python strings
        return str(obj)
    elif isinstance(obj, PyQtClass):
        #print("AlreadyWrapped")
        # already wrapped
        return obj
    elif obj and isinstance(obj, QObject):
        #print("Is QObject", obj.__class__.__name__, obj.metaObject().className())
        if force or obj.__class__.__name__ != obj.metaObject().className():
            # Ah this is an unwrapped class
            obj = create_pyqt_object(obj)
    return obj


def is_wrapped(obj):
    """
    checks if a object is wrapped by PyQtClass
    """
    # XXX: Any better/faster check?
    return hasattr(obj, "qt")



def unwrap(obj):
    """
    if wrapped returns the wrapped object
    """
    if is_wrapped(obj):
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
    Base error classed. Catch this to handle exceptions comming from C++
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
            #else:
            #    print("* deleting", qobj)
            # XXX: or better setdeleted ?
            sip.delete(qobj)
        #else:
        #    print("* NOT deleting", qobj)


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
        print("__getattr__", name)

        # Check whether the object is in the QObject hierarchy
        for child in self._instance.children():
            if str(child.objectName()) == name:
                print(name, "is a child:", child)
                obj = wrap(child)
                # save found object for faster lookup
                setattr(self, name, obj)
                return obj

        # Check whether it's a property
        v = self._instance.property(name)

        if (v is None):
            v = self._instance.property("b'" + name)

        print(v, dir(v))


        # Check whether the property
        if hasattr(v, '__class__') and issubclass(v.__class__, QObject):
            return wrap(v, True)

        if hasattr(v, '__type__') and not (v is None or v.type() is None) :
            return from_variant(v)

        return v




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
            # XXX: add unnamed childs?
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
        return from_variant(self.meta_property.read(obj._instance))


    def set(self, obj, value):
        self.meta_property.write(obj._instance, value)




class PyQtMethod(object):

    __slots__ = ["meta_method", "name", "args", "returnType", "__doc__"]


    def __init__(self, meta_method):
        self.meta_method = meta_method
        self.name, args = str(meta_method.methodSignature()).split("(", 1)
        self.args = args[:-1].split(",")
        self.returnType = str(meta_method.typeName())

        types = [str(t) for t in meta_method.parameterTypes()]
        names = [str(n) or "arg%i" % (i+1) \
                  for i, n in enumerate(meta_method.parameterNames())]
        params = ", ".join("%s %s" % (t, n) for n, t in zip(types, names))

        self.__doc__ = "%s(%s)%s" % (
           self.name, params,
           self.returnType and (" -> %s" % self.returnType) or ""
        )


    def instancemethod(self):
        def wrapper(obj, *args):
            # XXX: support kwargs?
            qargs = [Q_ARG(t, v) for t, v in zip(self.args, args)]
            invoke_args = [obj._instance, self.name]
            invoke_args.append(Qt.DirectConnection)
            rtype = self.returnType
            if rtype:
                invoke_args.append(Q_RETURN_ARG(rtype))
            invoke_args.extend(qargs)
            try:
                result = QMetaObject.invokeMethod(*invoke_args)
                error_msg = str(qApp.property("MIKRO_EXCEPTION").toString())
                if error_msg:
                    # clear message
                    qApp.setProperty("MIKRO_EXCEPTION", "") # TODO: "" was QVariant(): check that it's correct (ale/20141002)
                    raise Exception(error_msg)
            except RuntimeError as e:
                raise TypeError(
                    "%s.%s(%r) call failed: %s" % (obj, self.name, args, e))
            return wrap(result)
        wrapper.__doc__ = self.__doc__
        return wrapper




# Cache on-the-fly-created classes for better speed
# XXX Should I use weak references?
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
        #import pdb; pdb.set_trace()
        prop_name = str(prop.name)
        #prop_name = prop_name[0].upper() + prop_name[1:]
        if prop.read_only:
            # XXX: write set-method which raises an error
            properties[prop_name] = attrs[prop_name] = property(prop.get, doc=prop.__doc__)
        else:
            properties[prop_name] = attrs[prop_name] = property(
                                                      prop.get, prop.set, doc=prop.__doc__)

    methods = attrs["__methods__"] = {}
    signals = attrs["__signals__"] = {}
    for i in range(metaobject.methodCount()):
        meta_method = metaobject.method(i)
        if meta_method.methodType() != QMetaMethod.Signal :
            method = PyQtMethod(meta_method)
            method_name = method.name
            if method_name in attrs:
                # There is already a property with this name
                # So append an underscore
                method_name += "_"
            instance_method = method.instancemethod()
            instance_method.__doc__ = method.__doc__
            methods[method_name] = attrs[method_name] = instance_method
        else :
            method_name = meta_method.name()
            signal_attrs = []
            #for i in range(meta_method.parameterCount()):
            #    typ = meta_method.parameterType(i)
            #    signal_attrs.append(typ)
            #import pdb; pdb.set_trace()
            # TODO: bind the signal (which is now unbound) to the c++ signal (we can bind them signal to signal, if signal to slot does not work)
            # TODO: make sure that the signal shows up as attribute in the created class
            properties[bytes(method_name).decode('ascii')] = pyqtSignal(meta_method.parameterTypes())

    # import pdb; pdb.set_trace()

    # Python is great :)
    # It can dynamically create a class with a base class and a dictionary
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





