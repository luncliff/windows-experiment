<#
.SYNOPSIS
    Update version of the Package.appxmanifest
.DESCRIPTION
    Update Package.Identity.Version field in the given .appxmanifest file

    * https://github.com/microsoft/devops-for-windows-apps/blob/master/azure-pipelines.yml
    * https://learn.microsoft.com/en-us/windows/apps/publish/publish-your-app/package-version-numbering?pivots=store-installer-msix

.EXAMPLE
    PS> update-appxmanifest.ps1 -ManifestPath '.\App1 (Package)\Package.appxmanifest' -Version $(./headver.ps1)
#>
using namespace System.Xml
param
(
    [Parameter(Mandatory = $true)][String]$Version, # x.y.z
    [Parameter(Mandatory = $true)][String]$ManifestPath
)

$Settings = New-Object XmlWriterSettings
$Settings.Indent = $true
$Settings.NewLineOnAttributes = $true
$Settings.OmitXmlDeclaration = $false

# Load manifest contents before creating the XmlWriter
[Xml]$Manifest = Get-Content $ManifestPath
$Manifest.Package.Identity.Version = "$Version.0"

[XmlWriter]$Writer = [XmlWriter]::Create($ManifestPath, $Settings)
$Manifest.WriteContentTo($Writer)
$Writer.Flush()
$Writer.Close()
