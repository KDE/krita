import os
import site
import sysconfig

framework_path = os.path.dirname(os.path.abspath(__file__))
data_path = sysconfig.get_paths()['data']
# https://github.com/pypa/setuptools/issues/2165 (windows)
site.addsitedir(framework_path)
site.addsitedir(os.path.join(data_path, 'site-packages'))
site.addsitedir(os.path.join(framework_path,'site-packages'))
site.addsitedir(os.path.join(framework_path,'site-packages', 'PyQt5'))
