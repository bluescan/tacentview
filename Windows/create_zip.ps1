$file = "../Src/Version.h"

foreach($line in (Get-Content $file))
{
	$splitLine = $line.Split("=")
	if ($splitLine -and ($splitLine.Count -eq 2) )
	{
		$names = $splitLine.split(" `n`t",[System.StringSplitOptions]::RemoveEmptyEntries)
		$name = $names[2]
		$value = $splitLine[1].Trim(" ", "`n", "`t", ";")
		New-Variable -Name $name -Value $value
	}
}

$VersionString = $MajorVersion + "." + $MinorVersion + "." + $Revision
Write-Output $VersionString

$PackageName="tacittexview_${VersionString}"
Write-Output $PackageName
if (Test-Path $PackageName)
{
	Remove-Item $PackageName -Recurse
}
New-Item $PackageName -itemtype directory
New-Item "$($PackageName)\\Data" -itemtype directory

Copy-Item -Path "..\\TacitTexView.exe" -Destination "$($PackageName)\\"

# png txt ttf 
Copy-Item -Path "..\\Data\\*.png" -Destination "$($PackageName)\\Data"
Copy-Item -Path "..\\Data\\*.txt" -Destination "$($PackageName)\\Data"
Copy-Item -Path "..\\Data\\*.ttf" -Destination "$($PackageName)\\Data"

$ZipFile = "$($PackageName).zip"
if (Test-Path $ZipFile)
{
	Remove-Item $ZipFile
}
Compress-Archive -Path $PackageName -DestinationPath $ZipFile
