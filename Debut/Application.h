#pragma once

namespace Debut
{
	class _declspec(dllexport) Application
	{
	public:
		Application();
		
		virtual ~Application();

		void Run();
	};
}
