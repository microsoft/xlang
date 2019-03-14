import setuptools

setuptools.setup(
    name = "%",
    version = "1.0a0",
    description="Generated Python/WinRT package",
    license="MIT",
    url="http://github.com/Microsoft/xlang",
    ext_modules = [ setuptools.Extension('_%', 
        sources = [%],
        # TODO: add /permissive- when pywinrt moves to a later C++/WinRT version
        extra_compile_args = ["/std:c++17", "/await", "/GR-"], 
        include_dirs = ['.'],
        extra_link_args=['/MAP', '/DEBUG:FULL', '/OPT:REF', '/OPT:ICF'],
        libraries = ['windowsapp']) ],
    packages = setuptools.find_namespace_packages())
