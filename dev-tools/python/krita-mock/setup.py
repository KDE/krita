from setuptools import setup, find_namespace_packages

setup(
    name="krita",
    version="0.1",
    packages=find_namespace_packages('src'),
    package_dir={'': 'src'},
)
