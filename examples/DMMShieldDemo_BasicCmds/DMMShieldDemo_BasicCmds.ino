#include <DMMShield.h>


DMMShield dmmShieldObj;
uint8_t bErrCode = 0;

// the setup function runs once when you press reset or power the board
void setup() 
{
	Serial.begin(9600);
	dmmShieldObj.begin(&Serial);
	Serial.println("DMMShield Library Basic Commands demo");
	dmmShieldObj.ProcessIndividualCmd("DMMSetScale VoltageDC5");
//	bErrCode = dmmShieldObj.SetScale(8); 	//"5 V DC" scale
	if(bErrCode == 0)
	{
		// success
		Serial.println("5 V DC Scale");
	}
}

// the loop function runs over and over again forever
void loop() 
{
	char szMsg[20];
	while(1)
	{
		delay(1000);                       // wait one second
		bErrCode = dmmShieldObj.GetFormattedValue(szMsg);
		if(bErrCode == 0)
		{
			// success
			Serial.print("Value = ");
			Serial.println(szMsg);
		}
	}
}
