#include "engine/vs_app.h"
#include <iostream>
#include <cstdlib>

int main()
{
	vs::vs_app app{};

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}
