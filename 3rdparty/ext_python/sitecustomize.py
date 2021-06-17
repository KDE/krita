import os
import site
import sysconfig

# https://github.com/pypa/setuptools/issues/2165

if os.name == "nt":
    import pathlib
    framework_path = pathlib.Path(__file__).parent.parent
else:
    framework_path = os.path.dirname(os.path.abspath(__file__))
data_path = sysconfig.get_paths()['data']
site.addsitedir(os.path.join(data_path, 'site-packages'))
site.addsitedir(os.path.join(framework_path,'site-packages'))
site.addsitedir(os.path.join(framework_path,'site-packages', 'PyQt5'))
