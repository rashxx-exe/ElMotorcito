#include "Motor.h"

using namespace ElMotorcito;

int main(int, char**)
{
	ElMotorcito::Motor& motor = ElMotorcito::Motor::Instance();

	if (motor.Init() != 0)
	{
		return -1;
	}

	motor.Run();
	motor.Exit();

	return 0;
}
