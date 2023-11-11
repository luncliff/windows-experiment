<#
.SYNOPSIS
    Run MSBuild to gerenate .msixbundle

.EXAMPLE
    PS> publish.ps1 -WapProjectPath '.\App1\App1.wapproj' -OutDir "$(Get-Location)/outputs"
#>
param
(
    [Parameter(Mandatory = $true)][String]$OutDir,
    [String]$Configuration = "Debug",
    [Parameter(Mandatory = $true)][String]$WapProjectPath
)

MSBuild $WapProjectPath `
    /nologo /nodeReuse:true `
    /p:platform="x64" /p:configuration="$Configuration" `
    /p:AppxBundle=Always `
    /p:UapAppxPackageBuildMode=Sideload `
    /p:AppxBundlePlatforms="x64" `
    /p:AppxPackageDir="$OutDir" `
    /p:OutDir="$OutDir"
