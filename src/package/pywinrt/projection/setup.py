import setuptools

# Extract the build revision information from Build_BuildNumber environment variable. 
# This relies on the format of the pipeline name being of the format: <build text>.$(Date:yy).$(Date:MMdd).$(DayOfYear).$(Rev:r)

import os
buildNumber = os.environ["Build_BuildNumber"].split('.')
year = buildNumber[1]
doy = buildNumber[3]
rev = buildNumber[4]

setuptools.setup(
    name = "winrt",
    version = "1.0.{0}{1}.{2}".format(year, doy, rev),
    description="Generated Python/WinRT package",
    license="MIT",
    author='Microsoft Corporation',
    url="http://github.com/Microsoft/xlang",
    classifiers=[
            'Development Status :: 4 - Beta',
            'Environment :: Win32 (MS Windows)',
            'License :: OSI Approved :: MIT License',
            'Operating System :: Microsoft :: Windows :: Windows 10',
            'Programming Language :: Python :: 3.7',
            'Programming Language :: Python :: Implementation :: CPython',
            'Topic :: System :: Operating System',
        ],
    package_data={ "winrt":["_winrt.pyd"] },
    packages = setuptools.find_namespace_packages(where='.', include=("winrt*")))
