#include <yak.h>
#include "lab7defs.h"

ISR(INT4_vect, ISR_NOBLOCK) {
	YKEnterISR();
	sei();
	buttonHandler();
	cli();
	YKExitISR();
}

ISR(TIMER3_OVF_vect, ISR_NOBLOCK) {
	YKEnterISR();
	sei();
	YKTickHandler();
	cli();
	YKExitISR();
}

void buttonHandler() {
	int num = getNumber();

	if(num == 7) {
		YKEventSet(charEvent, EVENT_A_KEY);
	}
	else if(num == 8) {
		YKEventSet(charEvent, EVENT_B_KEY);
	}
	else if(num == 9) {
		YKEventSet(charEvent, EVENT_C_KEY);
	}
	else if(num == 5) {
		YKEventSet(charEvent, EVENT_A_KEY | EVENT_B_KEY | EVENT_C_KEY);
	}
	else if(num == 1) {
		YKEventSet(numEvent, EVENT_1_KEY);
	}
	else if(num == 2) {
		YKEventSet(numEvent, EVENT_2_KEY);
	}
	else if(num == 3) {
		YKEventSet(numEvent, EVENT_3_KEY);
	}
}

int getNumber() {
	int i = 0;
	int num = 0;
	uint16_t touchstatus;

	touchstatus = mpr121Read(0x01) << 8;
	touchstatus |= mpr121Read(0x00);

	if (touchstatus & (1<<SEVEN))
	num = 7;
	else if (touchstatus & (1<<FOUR))
	num = 4;
	else if (touchstatus & (1<<ONE))
	num = 1;
	else if (touchstatus & (1<<EIGHT))
	num = 8;
	else if (touchstatus & (1<<FIVE))
	num = 5;
	else if (touchstatus & (1<<TWO))
	num = 2;
	else if (touchstatus & (1<<NINE))
	num = 9;
	else if (touchstatus & (1<<SIX))
	num = 6;
	else if (touchstatus & (1<<THREE))
	num = 3;

	return num;
}