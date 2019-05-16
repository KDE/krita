import os
import site

framework_path = os.path.dirname(os.path.abspath(__file__))
site.addsitedir(os.path.join(framework_path,'site-packages'))
site.addsitedir(os.path.join(framework_path,'site-packages', 'PyQt5'))

