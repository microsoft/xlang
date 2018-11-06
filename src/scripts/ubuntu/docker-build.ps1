param (
    [string]$dockerImageRepoName = "xlang-ubuntu-build",
    [string]$dockerContainerRegistry = "msftxlangtemp.azurecr.io"
)

docker build -t $dockerImageRepoName $PSScriptRoot
docker tag $dockerImageRepoName "$dockerContainerRegistry/$dockerImageRepoName"
docker push "$dockerContainerRegistry/$dockerImageRepoName"
