#include "Controller.h"
#include "Dbms.h"
#include "UserInterface.h"


int main()
{
	Dbms dbms;
	Controller controller(&dbms);
	UserInterface ui(&controller);

	dbms.set_p_controller(&controller);
	controller.set_p_ui(&ui);

	ui.show_main_menu();

	return 0;
}