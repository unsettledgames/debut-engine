pushd .\..\Debut
mkdir Build
call cmake -G "Visual Studio 17 2022" -DBOX2D_BUILD_TESTBED=OFF
popd
PAUSE