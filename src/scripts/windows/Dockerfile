# escape=`

# Dockerfile based on https://github.com/Microsoft/vs-dockerfiles 

# make sure to use base image already cached in azure pipeline build image
# FROM microsoft/dotnet-framework@sha256:cfcd42bdc1646318cb8a314c298e8ba0b7e962829460c49ba2c609eb125e5e7f
FROM microsoft/dotnet-framework:4.7.2-runtime-windowsservercore-1803

ADD https://aka.ms/vs/15/release/vs_buildtools.exe C:\TEMP\vs_buildtools.exe
ADD https://go.microsoft.com/fwlink/p/?LinkID=2023014 c:\TEMP\winsdksetup.exe

# https://docs.microsoft.com/en-us/visualstudio/install/workload-component-id-vs-build-tools?view=vs-2017
RUN C:\TEMP\vs_buildtools.exe --quiet --wait --norestart --nocache `
    --add Microsoft.VisualStudio.Workload.VCTools `
    --add Microsoft.VisualStudio.Component.VC.CMake.Project `
    --add Microsoft.VisualStudio.Component.VSSDKBuildTools `
    --add Microsoft.VisualStudio.Component.VC.ATL `
    --add Microsoft.Net.Component.4.6.1.SDK `
    --installPath C:\BuildTools

RUN C:\TEMP\winsdksetup.exe /quiet /norestart /features OptionId.DesktopCPPx86 OptionId.DesktopCPPx64
