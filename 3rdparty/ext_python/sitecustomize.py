import os
import site
import sysconfig

framework_path = os.path.dirname(os.path.abspath(__file__))
data_path = sysconfig.get_paths()['data']
site.addsitedir(os.path.join(data_path, 'site-packages'))
site.addsitedir(os.path.join(framework_path,'site-packages'))
site.addsitedir(os.path.join(framework_path,'site-packages', 'PyQt5'))

