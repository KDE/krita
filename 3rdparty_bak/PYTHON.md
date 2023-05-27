Notes About Python And Windows

On Windows, we need two Pythons. One to configure Qt and build sip and PyQt5 against, one to package together with Krita, PyQt, sip and PyKrita. These Pythons need to be exactly the same version, and that version is specified in many places:


krita\CMakeLists.txt (find_package(PythonLibrary 3.8))
krita\3rdparty\ext_python\CMakeLists.txt (URL https://www.python.org/ftp/python/3.8.1/python-3.8.1-embed-amd64.zip)
krita\plugins\extensions\pykrita\plugin\utilities.cpp (paths.append(pythonDir.absoluteFilePath("python38.zip"));)

