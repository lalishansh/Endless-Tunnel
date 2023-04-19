param(
    # Platforms
    [switch]$Android,
    [switch]$Windows,
    [switch]$Linux,

    # Build-Type
    [switch]$Release #else Debug
    )

Write-Host "Build: Android:$Android, Windows:$Windows, Linux:$Linux | Release-build:$Release"
if (-not (Test-Path $PSScriptRoot/../out)){
    mkdir "out"
}
$OutDirectory = (Resolve-Path "$PSScriptRoot/../out").ToString()

$ProjectRoot = (Resolve-Path "$PSScriptRoot/../runtime/env_android").ToString()

if ($Android) {
    $WSL_port = "127.0.0.1:58526"
    $pckg_name = "imgui.example.android"

    if (Test-Path "$OutDirectory/android") {
        Resolve-Path "$OutDirectory/android/**/*-cmake" | Remove-Item -recurse -force
        Resolve-Path "$OutDirectory/android" | Remove-Item -recurse -force
    }

    $out_dir = "$OutDirectory/android/app-src"

    Push-Location "$ProjectRoot"
    $GoodToGo = $false
    try {
        adb connect $WSL_port
        if ($Release) {
            ./gradlew assembleRelease && ($GoodToGo = $true)
        } else {
            ./gradlew assembleDebug && ($GoodToGo = $true)
        }
        if ($GoodToGo) {
            adb -s $WSL_port install (Get-Item -Path "$out_dir/outputs/apk/*/*.apk").ToString() && adb -s $WSL_port logcat -c && adb -s $WSL_port shell monkey -p $pckg_name 1 #&& adb -s $WSL_port logcat
        }
    } catch { Write-Error "ERROR: $_" }
    finally {
        Pop-Location
    }
} 
$ProjectRoot = (Resolve-Path "$PSScriptRoot/..").ToString()
if ($Windows) {
    $cmake_build_type = "Debug"
    if ($Release) {
        $cmake_build_type = "Release"
    }
    $cmake_preset = "Windows x64 - Visual Studio 17 - MSVC @ $cmake_build_type"
    $out_dir = "$OutDirectory/$cmake_preset"
    $bin_dir = "$out_dir/$cmake_build_type"

    Push-Location "$ProjectRoot"
    $GoodToGo = $false
    try {
        cmake --preset "$cmake_preset" && cmake --build "$out_dir" && ($GoodToGo = $true)

        if ($GoodToGo) {
            Push-Location "$bin_dir"

            $exec_path = (Resolve-Path "*.exe" -Relative)
            if ($exec_path -is [System.Object[]]) {
                $exec_path = $exec_path[0]
            }

            $binary = $exec_path.ToString()

            Write-Host "`n## Running $out_dir $Binary ##`n"
            Invoke-Expression "$Binary"
        }
    } catch { Write-Error "ERROR: $_"}
    finally {
        if ($GoodToGo) { Pop-Location }
        Pop-Location
    }
}

($ProjectRoot.replace("\","/")) -match '^(\w)(:)(.+)'
$ProjectRoot = "/mnt/{0}{1}" -f $matches[1].ToLower(),$matches[3]
($OutDirectory.replace("\","/")) -match '^(\w)(:)(.+)'
$OutDirectory = "/mnt/{0}{1}" -f $matches[1].ToLower(),$matches[3]
if ($Linux) {
    $WSL_distro = "ubuntu"
    $RUN_command = "run"
    $PWSH_run_command = "pwsh -ExecutionPolicy ByPass -EncodedCommand"

    $cmake_build_type = "Debug"
    if ($Release) {
        $cmake_build_type = "Release"
    }
    $cmake_preset = "Linux x64 - Ninja - GNU @ $cmake_build_type"

    $out_dir = "$OutDirectory/$cmake_preset"
    $bin_dir = "$out_dir"

    $invoke_command = @"
        Push-Location `'$ProjectRoot`'
        `$GoodToGo = `$false
        try {
            cmake --preset `"$cmake_preset`" && cmake --build `"$out_dir`" && (`$GoodToGo = `$true)
            if (`$GoodToGo) {
                Push-Location `"$bin_dir`"
                `$binary = (@(find . -executable -type f) | Sort-Object)[0]
                Invoke-Expression `"$binary`"
            }
        } catch { Write-Error "ERROR: `$_" }
        finally {
            if (`$GoodToGo) {
                Pop-Location
            }
            Pop-Location
        }
"@

    Write-Host "$WSL_distro $RUN_command $PWSH_run_command `'`n$invoke_command`'"
    $bytes = [Convert]::ToBase64String([System.Text.Encoding]::Unicode.GetBytes($invoke_command))
    Invoke-Expression "$WSL_distro $RUN_command $PWSH_run_command `'$bytes`'"
}
