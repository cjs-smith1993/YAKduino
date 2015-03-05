#include <SerialGraphicLCD.h>

int num = 0;
LCD lcd;

void setup() {
	delay(5000);
	lcd.clearScreen();
}

void loop() {
	// lcd.printStr(A[num++]);
	// if (num %2 == 0) {
	// 	lcd.clearScreen();
	// }
	// else {
	// 	lcd.nextLine();
	// }
	// if (num >= sizeof(A)/sizeof(A[0])) {
	// 	num = 0;
	// }
	lcd.clearScreen();
	lcd.printNum(num++);
	lcd.drawCircle(30+num, 30+num/2, num, 1);
	if (num > 20) {
		num = 0;
	}
	delay(2000);
}
