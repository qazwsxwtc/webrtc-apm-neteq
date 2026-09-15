param(
    [Parameter(Mandatory=$true)][string]$BuildDir,
    [Parameter(Mandatory=$true)][string]$SourceDir
)

$ErrorActionPreference = "Stop"

$filtersFiles = Get-ChildItem -Path $BuildDir -Filter "*.vcxproj.filters" -File
if (-not $filtersFiles) {
    Write-Host "No .vcxproj.filters files found in $BuildDir"
    return
}

$rootPath = (Resolve-Path $SourceDir).Path.TrimEnd('\', '/')

function Get-RelativeFilterPath {
    param([string]$FullPath, [string]$RootPath, [string]$BaseType)

    $root = $RootPath.TrimEnd('\', '/') + '\'
    if (-not $FullPath.StartsWith($root, [System.StringComparison]::OrdinalIgnoreCase)) {
        return $BaseType
    }

    $rel = $FullPath.Substring($root.Length)
    $dir = Split-Path $rel -Parent

    if ([string]::IsNullOrEmpty($dir)) {
        return $BaseType
    }

    $normDir = $dir -replace '/', '\'
    return "$BaseType\$normDir"
}

$uniqueFilters = New-Object 'System.Collections.Generic.HashSet[string]'
[void]$uniqueFilters.Add("Source Files")
[void]$uniqueFilters.Add("Header Files")

$filterGuidMap = @{
    "Source Files" = '{5B4785AF-90C6-3832-9E9D-85F358DF3661}'
    "Header Files" = '{BA08A4E6-0A00-3904-99B9-30FDC548A827}'
}

foreach ($ff in $filtersFiles) {
    [xml]$xml = [System.IO.File]::ReadAllText($ff.FullName)
    $changed = $false

    foreach ($itemGroup in $xml.Project.ItemGroup) {
        if ($itemGroup.ClCompile) {
            foreach ($node in @($itemGroup.ClCompile)) {
                $inc = $node.Include
                $newFilter = Get-RelativeFilterPath -FullPath $inc -RootPath $rootPath -BaseType "Source Files"

                if ($node.Filter -ne $newFilter) {
                    $node.Filter = $newFilter
                    $changed = $true
                }
                [void]$uniqueFilters.Add($newFilter)
            }
        }
        if ($itemGroup.ClInclude) {
            foreach ($node in @($itemGroup.ClInclude)) {
                $inc = $node.Include
                $newFilter = Get-RelativeFilterPath -FullPath $inc -RootPath $rootPath -BaseType "Header Files"

                if ($node.Filter -ne $newFilter) {
                    $node.Filter = $newFilter
                    $changed = $true
                }
                [void]$uniqueFilters.Add($newFilter)
            }
        }
    }

    if ($changed) {
        $xml.Save($ff.FullName)
        Write-Host "Updated: $($ff.Name)"
    } else {
        Write-Host "Unchanged: $($ff.Name)"
    }
}

$filterList = @()
foreach ($f in $uniqueFilters) { $filterList += $f }
$filterList = $filterList | Sort-Object

foreach ($f in $filterList) {
    if (-not $filterGuidMap.ContainsKey($f)) {
        $filterGuidMap[$f] = '{' + [guid]::NewGuid().ToString().ToUpper() + '}'
    }
}

foreach ($ff in $filtersFiles) {
    [xml]$xml = [System.IO.File]::ReadAllText($ff.FullName)
    $filterItemGroup = $null
    $itemGroups = @($xml.Project.ItemGroup)
    foreach ($itemGroup in $itemGroups) {
        if ($itemGroup.Filter -and @($itemGroup.Filter).Count -gt 0 -and $itemGroup.Filter[0].Include) {
            $filterItemGroup = $itemGroup
            break
        }
    }

    if ($null -eq $filterItemGroup) {
        $filterItemGroup = $xml.CreateElement("ItemGroup")
        [void]$xml.Project.AppendChild($filterItemGroup)
    }

    $filtersToRemove = @($filterItemGroup.Filter)
    foreach ($oldFilter in $filtersToRemove) {
        if ($oldFilter -and $oldFilter.Include) {
            [void]$filterItemGroup.RemoveChild($oldFilter)
        }
    }

    foreach ($f in $filterList) {
        $filterElem = $xml.CreateElement("Filter")
        $filterElem.SetAttribute("Include", $f)

        $uidElem = $xml.CreateElement("UniqueIdentifier")
        $uidElem.InnerText = $filterGuidMap[$f]
        [void]$filterElem.AppendChild($uidElem)

        [void]$filterItemGroup.AppendChild($filterElem)
    }

    $xml.Save($ff.FullName)
}

Write-Host "Done. Total filters: $($filterList.Count)"