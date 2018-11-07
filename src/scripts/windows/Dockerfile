# escape=`

# Dockerfile based on https://github.com/Microsoft/vs-dockerfiles 

# make sure to use base image already cached in azure pipeline build image
FROM microsoft/dotnet-framework@sha256:cfcd42bdc1646318cb8a314c298e8ba0b7e962829460c49ba2c609eb125e5e7f

# Reset the shell.
SHELL ["cmd", "/S", "/C"]

# Set up environment to collect install errors.
COPY Install.cmd C:\TEMP\
ADD https://aka.ms/vscollect.exe C:\TEMP\collect.exe

# Install Node.js LTS
ADD https://nodejs.org/dist/v8.11.3/node-v8.11.3-x64.msi C:\TEMP\node-install.msi
RUN start /wait msiexec.exe /i C:\TEMP\node-install.msi /l*vx "%TEMP%\MSI-node-install.log" /qn ADDLOCAL=ALL

# Download channel for fixed install.
ARG CHANNEL_URL=https://aka.ms/vs/15/release/channel
ADD ${CHANNEL_URL} C:\TEMP\VisualStudio.chman

# Download and install Build Tools for Visual Studio 2017 for native desktop workload.
ADD https://aka.ms/vs/15/release/vs_buildtools.exe C:\TEMP\vs_buildtools.exe
RUN C:\TEMP\Install.cmd C:\TEMP\vs_buildtools.exe --quiet --wait --norestart --nocache `
    --channelUri C:\TEMP\VisualStudio.chman `
    --installChannelUri C:\TEMP\VisualStudio.chman `
    --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended`
    --installPath C:\BuildTools

# Use developer command prompt and start PowerShell if no other command specified.
ENTRYPOINT C:\BuildTools\Common7\Tools\VsDevCmd.bat &&
CMD ["powershell.exe", "-NoLogo", "-ExecutionPolicy", "Bypass"]
