pushd .\..
mkdir Build
cd Build
call cmake -G "Visual Studio 17 2022" -DWINDOWS_BUILD=ON .\..
copy .\..\Debut\vendor\mono\bin\mono-2.0-sgen.dll Debutant\Build\Release
copy .\..\Debut\vendor\mono\bin\mono-2.0-sgen.dll Debutant\Build\Debug
copy .\..\Debut\vendor\mono\bin\MonoPosixHelper.dll Debutant\Build\Release
copy .\..\Debut\vendor\mono\bin\MonoPosixHelper.dll Debutant\Build\Debug
copy .\DebutScripting\Build\Release\DebutScripting.dll Debutant\Build\Release
copy .\DebutScripting\Build\Debug\DebutScripting.dll Debutant\Build\Debug
popd
PAUSE