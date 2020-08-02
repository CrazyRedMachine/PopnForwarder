#define WINVER 0x0500
#include <windows.h>
#include <fstream>
#include <time.h>
#define POLL_RATE 10
//#define STATEDEBUG 1

//types for run time ezusb dll procedures call
typedef int (*start)(int);
typedef int (*firm)(void);
typedef int (*padread)(unsigned long*);
typedef int (*padreadlast)(unsigned char*);

typedef struct padstate_s {
	unsigned int button_state = 0;
	padread usbPadRead;
} padstate_t;

typedef struct lampstate_s {
	unsigned int prev_button_state = 0;
	uint8_t neon_anim_index = 0;
	uint8_t pillar_state_index = 0;
	bool pillar_lit = false;
	LARGE_INTEGER frequency;
	LARGE_INTEGER lastBlink;
	LARGE_INTEGER lastNeonUpdate;
	LARGE_INTEGER lastButtonAction;
	unsigned long actionRate = 0;
	unsigned long neonRate = 200;
	uint32_t lamp_bitfield = 0;
	start usbLamp;
} lampstate_t;

/*
bit 1 (LSB) to bit 5 : neon
bit 9       to bit 12: side lamps
bit 24      to bit 32: button lamps
 * */
uint32_t neon_anim[] = {16, 24, 28, 30, 31, 30, 28, 24};
uint32_t pillar_state[] = {0, 0xA00, 0x500, 0xF00};
void process_lamp(lampstate_t *lampState, unsigned int button_state){
	LARGE_INTEGER currTime;
	QueryPerformanceCounter(&currTime);
	currTime.QuadPart *= 1000;
	currTime.QuadPart /= lampState->frequency.QuadPart;

	uint32_t bitfield = (button_state & 0x1FF)<<23;
	unsigned int prevState = lampState->prev_button_state;
	lampState->prev_button_state = button_state;
	/*
	 * SIDE PILLARS
	 * When pressing any button the side pillars will blink for half a second
	 * The color is randomly chosen with blue being predominant, red rare and purple super rare
	 */
	if ( button_state != prevState )
	{
		if ( button_state != 0 ){
			long randNumber = rand()%21;
			if (randNumber == 0)
			{
				lampState->pillar_state_index = 3;
			}
			else if (randNumber < 3)
			{
				lampState->pillar_state_index = 1;
			} else {
				lampState->pillar_state_index = 2;
			}
			lampState->pillar_lit = true;
			if (button_state != 0) lampState->lastButtonAction.QuadPart = currTime.QuadPart;
		}
		prevState = button_state;
	} else { /* no state change, continue to blink for 50ms */
		if (currTime.QuadPart - (lampState->lastBlink).QuadPart > 50) {
			lampState->pillar_lit = !(lampState->pillar_lit);
			lampState->lastBlink.QuadPart = currTime.QuadPart;
		}
		if (currTime.QuadPart - (lampState->lastButtonAction).QuadPart > 500) {
			lampState->pillar_state_index = 0;
		}
	}

	if (lampState->pillar_lit)
		bitfield |= pillar_state[lampState->pillar_state_index];

	/*
	 * Adjusting top neon animation speed (should go faster when you hit buttons quickly)
	 */
	lampState->actionRate = currTime.QuadPart - lampState->lastButtonAction.QuadPart;

	if (15*lampState->actionRate < lampState->neonRate)
		lampState->neonRate *= 0.99995;
	else if (lampState->actionRate > 10*lampState->neonRate){
		lampState->neonRate = lampState->neonRate*1.01;
		if (lampState->neonRate < 100) lampState->neonRate++;
	}
	if (lampState->neonRate > 400)
		lampState->neonRate = 400;

	if (lampState->neonRate < 40)
		lampState->neonRate = 40;

	/*
	 * Cycling through the top neon animation
	 */
	if ((currTime.QuadPart - lampState->lastNeonUpdate.QuadPart) > lampState->neonRate)
	{
		lampState->neon_anim_index++;
		if (lampState->neon_anim_index > 7) lampState->neon_anim_index = 0;
		lampState->lastNeonUpdate.QuadPart = currTime.QuadPart;
	}

	bitfield |= neon_anim[lampState->neon_anim_index];

	/*
	 * Light the leds
	 */
	//neon_lights(neons);
	lampState->lamp_bitfield = bitfield;
#ifdef STATEDEBUG
	printf("LAMP : ");
	for (int i=0; i<32;i++){
		printf("%c",(bitfield>>31-i)&1?'1':'0');
	}
	printf("\n");
#endif
	//send lights
	lampState->usbLamp(lampState->lamp_bitfield);
}

/**
 * process panel inputs, generate keypresses accordingly and update the padstate struct.
 * @param padstate padstate struct containing the last processed buttonState and the usbPadRead function pointer
 */
void process_pad(padstate_t* padstate){
	unsigned long pad_bits = 0;
	unsigned int prevButtonState = padstate->button_state;
	//usbPadReadLast((unsigned char *) &pad_bits);
	padstate->usbPadRead(&pad_bits);
	//if (usbSetExtIo != NULL) usbSetExtIo(0);

	unsigned int buttonState;
    buttonState |= (pad_bits >> 8) & 0x1FF;
	buttonState |= ((pad_bits >> 6)& 0x01) << 9;
	buttonState |= ((pad_bits >> 7)& 0x01) << 10;
	buttonState |= ((pad_bits >> 22) & 0x01) << 11;

	if (buttonState != prevButtonState) {
#ifdef STATEDEBUG
		printf("padread got %u\n",pad_bits);
			printf("prevBuState = ");
	for (int i = 0; i<12;i++){
		printf("%c",(prevButtonState>>i)&1?'1':'0');
	}
	printf("\n");
	printf("buttonState = ");
	for (int i = 0; i<12;i++){
		printf("%c",(buttonState>>i)&1?'1':'0');
	}
	printf("\n");
	printf("buttonstate changed, now is %u\n",buttonState);
#endif
	    INPUT in[12] = {0}; // up to 12 state changes at once (buttons)
	    int input_index = 0;
	    for (int i = 0; i<12; i++) {
		    if ( ((buttonState >> i)&1) != ((prevButtonState >> i)&1) ){
#ifdef STATEDEBUG
	    		printf("button number %d has changed state\n",i+1);
#endif
		    	in[input_index].type = INPUT_KEYBOARD;
			    in[input_index].ki.wScan = '1'-0x2F+i;
			    in[input_index].ki.time = 0;
			    in[input_index].ki.dwExtraInfo = 0;
			    in[input_index].ki.wVk = 0;
			    in[input_index].ki.dwFlags = 0x0008;
			    if (!((buttonState >> i)&1))  in[input_index].ki.dwFlags |= 0x0002; // 0 for key press else key release
#ifdef STATEDEBUG
	    	    if (in[input_index].ki.dwFlags == 0x0008) printf("hold");
			else printf("release");
			printf(" at in[%d]\n",input_index);
#endif
			    input_index++;
		    }
	    }
#ifdef STATEDEBUG
	    printf("Gathered %d state changes\n",input_index);
#endif
	    SendInput(input_index, in, sizeof(INPUT));
	}
	padstate->button_state = buttonState;
}

int main(int argc, char* argv[])
{
/* LOAD REQUIRED MODULE */
	start usbStart;
	firm usbFirmResult;
	start usbCoinMode;
	start usbLamp;
	padreadlast usbPadReadLast;
	padread usbPadRead;
	start usbSetExtIo;
	firm usbEnd;

	printf("Loading module ezusb.dll...\n");

	HMODULE hinstLib = LoadLibrary("ezusb.dll");
 
#ifdef STATEDEBUG
	printf("ezusb.dll found at %x\n",hinstLib);
#endif
	if (hinstLib == NULL) {
		auto err = GetLastError();
		printf("ERROR: unable to load ezusb.dll (error %d)\n",err);
		return 1;
	}

	// Get functions pointers
	usbStart = (start)GetProcAddress(hinstLib, "?usbStart@@YAHH@Z");
	usbFirmResult = (firm)GetProcAddress(hinstLib, "?usbFirmResult@@YAHXZ");
	usbCoinMode = (start)GetProcAddress(hinstLib, "?usbCoinMode@@YAHH@Z");
	usbLamp = (start)GetProcAddress(hinstLib, "?usbLamp@@YAHH@Z");
	usbPadReadLast = (padreadlast)GetProcAddress(hinstLib, "?usbPadReadLast@@YAHPAE@Z");
	usbPadRead = (padread)GetProcAddress(hinstLib, "?usbPadRead@@YAHPAK@Z");
	usbSetExtIo = (start)GetProcAddress(hinstLib, "?usbSetExtIo@@YAHH@Z");
	usbEnd = (firm)GetProcAddress(hinstLib, "?usbEnd@@YAHXZ");
	
	printf("Loading functions\n");
#ifdef STATEDEBUG
	printf("function usbStart found at %x\n",usbStart);
	printf("function usbFirmResult found at %x\n",usbFirmResult);
	printf("function usbCoinMode found at %x\n",usbCoinMode);
	printf("function usbLamp found at %x\n",usbLamp);
	printf("function usbPadReadLast found at %x\n",usbPadReadLast);
	printf("function usbPadRead found at %x\n",usbPadRead);
	printf("function usbSetExtIo found at %x\n",usbSetExtIo);
	printf("function usbEnd found at %x\n",usbEnd);
#endif

	if (usbStart == NULL || usbFirmResult == NULL || usbCoinMode == NULL ||  usbLamp == NULL ||  usbPadReadLast == NULL ||  usbPadRead == NULL ) {
		printf("ERROR: a required function was not found in ezusb.dll\n");
		FreeLibrary(hinstLib);
		return 1;
	}

/* INITIALIZE IOBOARD */

	printf("Init complete, starting main loop\n");
	bool done = false;

	printf("usbStart...");
	while (usbStart(0) != 0){
	}
	printf("ok\n");
	printf("usbFirmResult...");
	while (usbFirmResult()!=0){}
	printf("ok\n");

	printf("usbCoinMode...");
	while (usbCoinMode(3)!=0){}
	printf("ok\n");
	Sleep(2000);
	printf("You can press buttons now!\n");

	//init padState
	srand(time(NULL));
	padstate_t padState;
	padState.usbPadRead = usbPadRead;

	//init lampState
	lampstate_t lampState;
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	lampState.frequency = freq;
	lampState.lastBlink.QuadPart = 0;
	lampState.lastNeonUpdate.QuadPart = 0;
	lampState.lastButtonAction.QuadPart = 0;
	lampState.usbLamp = usbLamp;

/* POLLING LOOP */

	while(!done){
		process_pad(&padState);
		process_lamp(&lampState, padState.button_state);
		Sleep(POLL_RATE);
	}
	
	return 0;
}
