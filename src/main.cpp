#include "application.h"

int main(void)
{
	Application app;

	app.init();
	app.run();
	app.shutdown();
  
    return 0;
}