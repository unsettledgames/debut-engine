#include <stdio.h>

namespace Debut
{
	_declspec(dllimport) void Print();
}

void main() 
{
	Debut::Print();
}