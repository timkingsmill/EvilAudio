param(
    [string]$SourceDir = $null,
    [string]$BuildDir = $null,
    [string]$OutputFile = $null
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptDir "..\..\..")

if ([string]::IsNullOrWhiteSpace($SourceDir)) {
    $SourceDir = $repoRoot.Path
}

if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $repoRoot "build"
}

if ([string]::IsNullOrWhiteSpace($OutputFile)) {
    $OutputFile = Join-Path $scriptDir "cmake-targets.puml"
}

$dotPath = Join-Path $BuildDir "targets.dot"

Write-Host "Generating CMake DOT graph..."
cmake -S $SourceDir -B $BuildDir --graphviz=$dotPath

if (-not (Test-Path $dotPath)) {
    throw "CMake graphviz output was not created: $dotPath"
}

$lines = Get-Content $dotPath
$nodeRegex = '"(node\d+)" \[ label = "([^"]+)", shape = ([a-z]+) \];'
$edgeRegex = '"(node\d+)" -> "(node\d+)"( \[ style = (\w+) \])?'
$nodes = @{}
$edges = @()

foreach ($line in $lines) {
    if ($line -match $nodeRegex) {
        $id = $matches[1]
        $rawLabel = $matches[2]
        $shape = $matches[3]
        $display = ($rawLabel -split '\\n')[0]
        $nodes[$id] = @{ label = $display; shape = $shape }
    }
    elseif ($line -match $edgeRegex) {
        $edges += @{ from = $matches[1]; to = $matches[2]; style = $matches[4] }
    }
}

$out = New-Object System.Collections.Generic.List[string]
$out.Add('@startuml cmake_targets')
$out.Add('!include ../../puml/themes/EvilTheme.puml')
$out.Add('title EvilAudio CMake Target Dependency Graph')
$out.Add('left to right direction')
$out.Add('skinparam linetype ortho')
$out.Add('skinparam defaultTextAlignment center')
$out.Add('skinparam rectangle {')
$out.Add('  RoundCorner 10')
$out.Add('  BorderColor #2b2b2b')
$out.Add('}')
$out.Add('skinparam stereotype {')
$out.Add('  CBackgroundColor<<executable>> #DFF5E3')
$out.Add('  CBorderColor<<executable>> #2E7D32')
$out.Add('  CBackgroundColor<<static_lib>> #E3F2FD')
$out.Add('  CBorderColor<<static_lib>> #1565C0')
$out.Add('  CBackgroundColor<<interface_lib>> #FFF8E1')
$out.Add('  CBorderColor<<interface_lib>> #F9A825')
$out.Add('  CBackgroundColor<<other>> #F3E5F5')
$out.Add('  CBorderColor<<other>> #6A1B9A')
$out.Add('}')

foreach ($id in ($nodes.Keys | Sort-Object { [int]($_ -replace 'node', '') })) {
    $node = $nodes[$id]
    $stereotype = switch ($node.shape) {
        'egg' { 'executable' }
        'octagon' { 'static_lib' }
        'pentagon' { 'interface_lib' }
        default { 'other' }
    }

    $label = $node.label -replace '"', '\\"'
    $out.Add(('rectangle "{0}" as {1} <<{2}>>' -f $label, $id, $stereotype))
}

$out.Add('')
$out.Add('legend right')
$out.Add('|= Type |= Meaning |')
$out.Add('|<back:#DFF5E3>Executable</back>|CMake executable target|')
$out.Add('|<back:#E3F2FD>Static Library</back>|CMake static library target|')
$out.Add('|<back:#FFF8E1>Interface Library</back>|CMake interface library target|')
$out.Add('|<back:#F3E5F5>Other</back>|Custom/unknown target type|')
$out.Add('|Private link|dotted arrow|')
$out.Add('|Interface link|dashed arrow|')
$out.Add('|Other link|solid arrow|')
$out.Add('endlegend')
$out.Add('')

foreach ($edge in $edges) {
    if (-not $nodes.ContainsKey($edge.from) -or -not $nodes.ContainsKey($edge.to)) {
        continue
    }

    $arrow = switch ($edge.style) {
        'dotted' { '..>' }
        'dashed' { '-->' }
        default { '->' }
    }

    $out.Add(('{0} {1} {2}' -f $edge.from, $arrow, $edge.to))
}

$out.Add('@enduml')

Set-Content -Path $OutputFile -Value $out -Encoding ascii
Write-Host ("Generated {0} with {1} targets and {2} edges." -f $OutputFile, $nodes.Count, $edges.Count)
