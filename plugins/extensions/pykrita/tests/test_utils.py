import sys


def setPykritaInstPath():
    """ The third argument in the cmake file must be the CMAKE_INST_DIR """

    sys.path.insert(0, str("{0}/share/krita/pykrita/PyKrita").format(sys.argv[2]))
