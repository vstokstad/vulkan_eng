 function CompileShaders($shader){
 glsc $shader.Name
 Write-Host "compiled" $shader.Name
 }

$shaders = Get-ChildItem -Path shaders -Exclude *.spv

forceah(:$shaders){
  CompileShaders $shader }




$compiled = Get-ChildItem -Filter *.spv
Write-Output -InputObject $compiled
