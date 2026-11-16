$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$build = Join-Path $root 'build'
$dist = Join-Path $root 'dist'

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere)) { throw 'Visual Studio 2022 Build Tools are required.' }
$vsInstall = & $vswhere -latest -products '*' -property installationPath
$msbuild = Join-Path $vsInstall 'MSBuild\Current\Bin\MSBuild.exe'
if (-not (Test-Path -LiteralPath $msbuild)) { throw 'MSBuild.exe was not found.' }

$solutionDir = $root.TrimEnd('\') + '\'
foreach ($project in @('Launcher\Launcher.vcxproj', 'Plugin\SimpleUnlocker.vcxproj', 'ThirdParty\DisplayCD\DisplayCD.vcxproj')) {
    & $msbuild (Join-Path $root $project) /p:Configuration=Release /p:Platform=x64 "/p:SolutionDir=$solutionDir" /v:minimal
    if ($LASTEXITCODE -ne 0) { throw "Native build failed: $project" }
}

$localDotnet = Join-Path $root '.dotnet-sdk\dotnet.exe'
$siblingDotnet = Join-Path $root '..\Litelauncher\.dotnet-sdk\dotnet.exe'
$dotnet = if (Test-Path -LiteralPath $localDotnet) { $localDotnet }
          elseif (Test-Path -LiteralPath $siblingDotnet) { $siblingDotnet }
          else { (Get-Command dotnet.exe -ErrorAction Stop).Source }
$sdkDir = Split-Path -Parent $dotnet
$env:DOTNET_ROOT = $sdkDir
$env:PATH = "$sdkDir;$env:PATH"
$env:DOTNET_CLI_HOME = Join-Path $build 'dotnet-home'
$cachedPackages = Join-Path $root '..\Litelauncher\.build-home\.nuget\packages'
$env:NUGET_PACKAGES = if (Test-Path -LiteralPath $cachedPackages) { $cachedPackages }
                      else { Join-Path $build 'nuget-packages' }
$env:APPDATA = Join-Path $build 'appdata'
$env:DOTNET_CLI_TELEMETRY_OPTOUT = '1'
New-Item -ItemType Directory -Force -Path $build, $dist, $env:APPDATA | Out-Null

$gui = Join-Path $root 'UnlockerGUI\UnlockerGUI.csproj'
$restoreSource = if (Test-Path -LiteralPath $cachedPackages) {
    $emptySource = Join-Path $build 'empty-nuget'
    New-Item -ItemType Directory -Force -Path $emptySource | Out-Null
    $emptySource
} else { 'https://api.nuget.org/v3/index.json' }
& $dotnet restore $gui -r win-x64 -p:Platform=x64 --source $restoreSource
if ($LASTEXITCODE -ne 0) { throw 'GUI restore failed.' }

foreach ($variant in @(@{Name='self-contained'; SelfContained='true'; File='LiteUnlocker-v1.1.2-win-x64-self-contained.exe'},
                       @{Name='runtime'; SelfContained='false'; File='LiteUnlocker-v1.1.2-win-x64.exe'})) {
    $output = Join-Path $build "publish-$($variant.Name)"
    & $dotnet publish $gui -c Release -r win-x64 --self-contained $variant.SelfContained -p:Platform=x64 -p:PublishSingleFile=true --no-restore -o $output
    if ($LASTEXITCODE -ne 0) { throw "GUI publish failed: $($variant.Name)" }
    Copy-Item -LiteralPath (Join-Path $output 'UnlockerGUI.exe') -Destination (Join-Path $dist $variant.File) -Force
    Write-Output (Join-Path $dist $variant.File)
}
