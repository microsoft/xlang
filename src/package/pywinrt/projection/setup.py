import setuptools

# distutils.util.get_platform

setuptools.setup(
    name = "winrt",
    version = "1.0a0",
    description="Generated Python/WinRT package",
    license="MIT",
    author='Microsoft Corporation',
    url="http://github.com/Microsoft/xlang",
    classifiers=[
            'Development Status :: 4 - Beta',
            'Environment :: Win32 (MS Windows)'
            'License :: OSI Approved :: MIT License',
            'Operating System :: Microsoft :: Windows :: Windows 10'
            'Programming Language :: Python :: 3.7',
            'Programming Language :: Python :: Implementation :: CPython'
            'Topic :: System :: Operating System',
        ],
    package_data={ "winrt":["_winrt.pyd"] },
    packages = setuptools.find_namespace_packages(where='.', include=("winrt*")))
