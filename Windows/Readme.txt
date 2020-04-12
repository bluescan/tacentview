You may need to run this to execute powershell scripts from the VS post build step.
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope LocalMachine
Set-ExecutionPolicy Unrestricted

Use the following to run a script from the post-build step in VS.
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -file thescript.ps1
