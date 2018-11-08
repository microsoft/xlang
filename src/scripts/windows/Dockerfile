# escape=`

# Dockerfile based on https://github.com/Microsoft/vs-dockerfiles 

# make sure to use base image already cached in azure pipeline build image
FROM microsoft/dotnet-framework@sha256:cfcd42bdc1646318cb8a314c298e8ba0b7e962829460c49ba2c609eb125e5e7f

# Reset the shell.
SHELL ["cmd", "/S", "/C"]

# download NodeJS and VS Build tool installers
# ARG NODE_VERSION=v8.12.0
ADD https://nodejs.org/dist/v8.12.0/node-v8.12.0-x64.msi https://aka.ms/vs/15/release/vs_buildtools.exe c:\TEMP\

# node required for running AzDO tasks
RUN start /wait msiexec.exe /i C:\TEMP\node-v8.12.0-x64.msi /l*vx "%TEMP%\MSI-node-install.log" /qn ADDLOCAL=ALL

# https://docs.microsoft.com/en-us/visualstudio/install/workload-component-id-vs-build-tools?view=vs-2017
RUN C:\TEMP\vs_buildtools.exe --quiet --wait --norestart --nocache `
    --add Microsoft.VisualStudio.Workload.VCTools `
    --add Microsoft.VisualStudio.Component.VC.CMake.Project `
    --add Microsoft.VisualStudio.Component.Windows10SDK.17663 `
    --add Microsoft.VisualStudio.Component.VSSDKBuildTools `
    --add Microsoft.VisualStudio.Component.VC.ATL `
    --installPath C:\BuildTools

# Use developer command prompt and start PowerShell if no other command specified.
ENTRYPOINT C:\BuildTools\Common7\Tools\VsDevCmd.bat &&
CMD ["powershell.exe", "-NoLogo", "-ExecutionPolicy", "Bypass"]
