param([string]$PortName = 'COM5')
$ErrorActionPreference = 'Stop'
$serial = $null
$writer = $null
try {
    $captureDir = Join-Path $PSScriptRoot 'captures'
    New-Item -ItemType Directory -Path $captureDir -Force | Out-Null
    $outputPath = Join-Path $captureDir ('readings-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff') + '.csv')
    $serial = New-Object System.IO.Ports.SerialPort $PortName,115200,None,8,One
    $serial.ReadTimeout = 50
    $serial.WriteTimeout = 1000
    $serial.NewLine = "`n"
    # UNO R4 USB CDC may withhold traffic until the host asserts DTR.
    $serial.DtrEnable = $true
    $serial.Open()
    $writer = New-Object System.IO.StreamWriter($outputPath, $false, (New-Object System.Text.UTF8Encoding($false)))
    $writer.AutoFlush = $true
    $header = 'trial,label,sample,t_ms,x_raw,y_raw,z_raw,marker'
    $writer.WriteLine($header)
    Write-Host "Connected to $PortName at 115200 baud."
    Write-Host "Saving to: $outputPath"
    Write-Host 'r = rest | g = full reps | p = partial reps | n = other movement'
    Write-Host 's = stop recording | m = optional marker | q = save and quit'
    Write-Host 'Press a command key here. No Enter needed. Close Serial Monitor first.'
    Write-Host 'Wait for Ready before starting a trial.'
    # Opening a port can reset the Arduino. Do not begin a trial until it boots.
    Start-Sleep -Seconds 2
    $serial.Write('s')
    $serial.Write('h')
    Write-Host 'Ready. Press r, g, p, or n to begin.'
    $rows = 0
    $lastReport = [DateTime]::Now
    $quitting = $false
    while (-not $quitting) {
        while ([Console]::KeyAvailable) {
            $key = [char]::ToLowerInvariant([Console]::ReadKey($true).KeyChar)
            if ($key -eq 'q') {
                $serial.Write('s')
                $quitting = $true
                break
            }
            if ('rgpnsmh'.Contains([string]$key)) {
                $serial.Write([string]$key)
                Write-Host "Sent: $key"
            }
        }
        try {
            $line = $serial.ReadLine().Trim()
            if ($line -match '^\d+,[rgpn],\d+,\d+,\d+,\d+,\d+,\d+$') {
                $writer.WriteLine($line)
                $rows++
            } elseif ($line -and $line -ne $header) {
                Write-Host "Device: $line"
            }
        } catch [System.TimeoutException] {
            # A stopped logger sends no data. Keep accepting keyboard commands.
        }
        if (([DateTime]::Now - $lastReport).TotalSeconds -ge 2) {
            Write-Host "Saved $rows readings"
            $lastReport = [DateTime]::Now
        }
    }
    # Drain pending complete rows after sending stop.
    $drainUntil = [DateTime]::Now.AddMilliseconds(500)
    while ([DateTime]::Now -lt $drainUntil) {
        try {
            $line = $serial.ReadLine().Trim()
            if ($line -match '^\d+,[rgpn],\d+,\d+,\d+,\d+,\d+,\d+$') {
                $writer.WriteLine($line)
                $rows++
            }
        } catch [System.TimeoutException] { }
    }
    Write-Host "Finished: $rows readings saved to $outputPath"
} catch {
    Write-Host "Logger stopped: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host 'Close Arduino Serial Monitor/Plotter and any other serial programs, then retry.'
    Write-Host 'Any readings already written remain in the CSV file.'
} finally {
    if ($serial -and $serial.IsOpen) {
        try { $serial.Write('s') } catch { }
        $serial.Close()
    }
    if ($writer) { $writer.Dispose() }
    if ($serial) { $serial.Dispose() }
}
