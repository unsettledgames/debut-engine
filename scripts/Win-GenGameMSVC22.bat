pushd .\..
mkdir Game\Build
cd Game\Build
call cmake -G "Visual Studio 17 2022" -DWINDOWS_BUILD=ON .\..
copy .\..\..\Resources\Mono\Debut\Debug\DebutScriptingd.dll .\Debug\DebutScripting.dll
copy .\..\..\Resources\Mono\Debut\Release\DebutScripting.dll .\Release
popd
PAUSE