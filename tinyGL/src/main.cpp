//tiny openGL project
#include "App.hpp"

using namespace Kong;

void main()
{
	KongApp app;
	try
	{

		app.Run();	
	}
	catch (std::exception &e)
	{
		throw std::runtime_error(std::string("Error: ") + e.what());
	}
}
