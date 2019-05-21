# escape=`

# Dockerfile based on https://github.com/Microsoft/vs-dockerfiles 

FROM microsoft/dotnet-framework:4.7.2-runtime-windowsservercore-1803

ADD https://aka.ms/vs/16/release/vs_buildtools.exe C:\TEMP\vs_buildtools.exe

# https://docs.microsoft.com/en-us/visualstudio/install/workload-and-component-ids?view=vs-2019
RUN C:\TEMP\vs_buildtools.exe --quiet --wait --norestart --nocache `
    --add Microsoft.VisualStudio.Workload.VCTools `
    --add Microsoft.VisualStudio.Component.Windows10SDK.17763 `
    --add Microsoft.VisualStudio.Component.VC.CMake.Project `
    --add Microsoft.VisualStudio.Component.VC.Tools.ARM `
    --add Microsoft.VisualStudio.Component.VC.Tools.ARM64 `
    --add Microsoft.VisualStudio.Component.VSSDKBuildTools `
    --add Microsoft.VisualStudio.Component.VC.ATL `
    --add Microsoft.Net.Component.4.6.1.SDK `
    --add Microsoft.Net.Component.4.5.2.TargetingPack `
    --installPath C:\BuildTools
