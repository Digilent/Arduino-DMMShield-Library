#include <DMMShield.h>


DMMShield dmmShieldObj;

// the setup function runs once when you press reset or power the board
void setup() 
{
	Serial.begin(9600);
	dmmShieldObj.begin(&Serial);
	Serial.println("DMMShield Library Command Interpreter Demo");
}

// the loop function runs over and over again forever
void loop() 
{
	dmmShieldObj.CheckForCommand();
	delay(500);                       // wait half a second
}
