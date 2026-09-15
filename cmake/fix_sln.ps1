param(
    [Parameter(Mandatory=$true)][string]$SlnPath
)

$ErrorActionPreference = "Stop"

$folderMap = @{
    'webrtc_apm' = 'webrtc'
    'webrtc_neteq' = 'webrtc'
    'rtc_base' = 'webrtc/base'
    'common_audio' = 'webrtc/base'
    'system_wrappers' = 'webrtc/base'
    'pffft' = 'webrtc/third_party'
    'fft' = 'webrtc/third_party'
    'rnn_vad' = 'webrtc/third_party'
    'jsoncpp' = 'webrtc/third_party'
    'webrtc_cng' = 'webrtc/codecs'
    'legacy_encoded_audio_frame' = 'webrtc/codecs'
    'isac_vad' = 'webrtc/codecs'
    'isac_c' = 'webrtc/codecs'
    'isac_fix_common' = 'webrtc/codecs'
    'isac_fix_c' = 'webrtc/codecs'
    'isac' = 'webrtc/codecs'
    'isac_fix' = 'webrtc/codecs'
    'basic_apm' = 'example'
    'aec_demo' = 'example'
    'agc_demo' = 'example'
    'ns_demo' = 'example'
    'vad_demo' = 'example'
    'apm_pipeline' = 'example'
    'hpf_aec_ns_agc_vad' = 'example'
}

$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
$lines = [System.IO.File]::ReadAllLines($SlnPath)

$projects = New-Object System.Collections.ArrayList
foreach ($line in $lines) {
    $m = [regex]::Match($line, '^\s*Project\("\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942\}"\)\s*=\s*"([^"]+)"\s*,\s*"[^"]+\.vcxproj"\s*,\s*"\{([0-9A-F-]+)\}"\s*$')
    if ($m.Success) {
        [void]$projects.Add([PSCustomObject]@{
            Name = $m.Groups[1].Value
            Guid = $m.Groups[2].Value
        })
    }
}

$uniqueFolders = New-Object 'System.Collections.Generic.HashSet[string]'
foreach ($kv in $folderMap.GetEnumerator()) {
    [void]$uniqueFolders.Add($kv.Value)
}
$tmp = @()
foreach ($f in $uniqueFolders) { $tmp += $f }
foreach ($f in $tmp) {
    $parts = $f -split '/'
    for ($i = 1; $i -lt $parts.Count; $i++) {
        [void]$uniqueFolders.Add(($parts[0..($i - 1)] -join '/'))
    }
}

$folderGuidMap = @{}
foreach ($f in $uniqueFolders) {
    $folderGuidMap[$f] = '{' + [guid]::NewGuid().ToString().ToUpper() + '}'
}

$slnFolderGuid = '{2150E333-8FDC-42A3-9474-1A3956D46DE8}'

$sb = New-Object System.Text.StringBuilder

$globalIdx = -1
for ($i = 0; $i -lt $lines.Count; $i++) {
    if ($lines[$i] -eq 'Global') {
        $globalIdx = $i
        break
    }
    [void]$sb.AppendLine($lines[$i])
}

$folderList = @()
foreach ($f in $uniqueFolders) { $folderList += $f }
$folderList = $folderList | Sort-Object

foreach ($folder in $folderList) {
    $guid = $folderGuidMap[$folder]
    $name = ($folder -split '/')[-1]
    [void]$sb.AppendLine("Project(`"$slnFolderGuid`") = `"$name`", `"$folder`", `"$guid`"")
    [void]$sb.AppendLine('EndProject')
}

[void]$sb.AppendLine('Global')
[void]$sb.AppendLine("`tGlobalSection(NestedProjects) = preSolution")

foreach ($folder in $folderList) {
    $guid = $folderGuidMap[$folder]
    $parentFolder = Split-Path $folder -Parent
    if (-not [string]::IsNullOrEmpty($parentFolder)) {
        $parentGuid = $folderGuidMap[$parentFolder]
        [void]$sb.AppendLine("`t`t$guid = $parentGuid")
    }
}

foreach ($proj in $projects) {
    if ($folderMap.ContainsKey($proj.Name)) {
        $folder = $folderMap[$proj.Name]
        $parentGuid = $folderGuidMap[$folder]
        [void]$sb.AppendLine("`t`t{$($proj.Guid)} = $parentGuid")
    }
}

[void]$sb.AppendLine("`tEndGlobalSection")

for ($i = $globalIdx; $i -lt $lines.Count; $i++) {
    [void]$sb.AppendLine($lines[$i])
}

[System.IO.File]::WriteAllText($SlnPath, $sb.ToString(), $utf8NoBom)
Write-Host "Fixed: $SlnPath (folders=$($uniqueFolders.Count), projects=$($projects.Count))"