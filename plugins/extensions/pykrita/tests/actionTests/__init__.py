import sys


inst_dir = sys.argv[2]
source_dir = sys.argv[3]
sys.path.insert(0, str("{0}/share/krita/pykrita/PyKrita").format(inst_dir))
