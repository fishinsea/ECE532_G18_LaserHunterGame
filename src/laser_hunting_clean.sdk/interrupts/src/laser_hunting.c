#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "glue.h"

#include <xuartlite_l.h>
#include <xintc_l.h>
#include <xparameters.h>
volatile unsigned int * mode = (unsigned int *)(XPAR_GLUE_0_S00_AXI_BASEADDR + 32);
volatile unsigned int * score_2_reg = (unsigned int *)(XPAR_GLUE_0_S00_AXI_BASEADDR + 36);
volatile unsigned int * timer_reg = (unsigned int *)(XPAR_GLUE_0_S00_AXI_BASEADDR + 44);
volatile unsigned int * audio_reg = (unsigned int *)(XPAR_GLUE_0_S00_AXI_BASEADDR + 40);
volatile unsigned int * x_reg = (unsigned int *)(XPAR_GLUE_0_S00_AXI_BASEADDR);
volatile unsigned int * y_reg = (unsigned int *)(XPAR_GLUE_0_S00_AXI_BASEADDR + 4);
volatile unsigned int * x_reg_green = (unsigned int *)(XPAR_GLUE_0_S00_AXI_BASEADDR + 8);
volatile unsigned int * y_reg_green = (unsigned int *)(XPAR_GLUE_0_S00_AXI_BASEADDR + 12);
volatile unsigned int * select_reg = (unsigned int *)(XPAR_GLUE_0_S00_AXI_BASEADDR + 48);
volatile unsigned int * score_reg = (unsigned int *)(XPAR_GLUE_0_S00_AXI_BASEADDR + 52);
volatile unsigned int * x_target_reg = (unsigned int *)(XPAR_GLUE_0_S00_AXI_BASEADDR + 56);
volatile unsigned int * y_target_reg = (unsigned int *)(XPAR_GLUE_0_S00_AXI_BASEADDR + 60);
int paused = 0;
int gamelength = 10000;
int timer = 0;
int currenttime = 99;
int start = 0;
int diff = 1;
int period = 50;

/* uartlite interrupt service routine */
void uart_int_handler(void *baseaddr_p) {
	char c;
	/* till uart FIFOs are empty */
	while (!XUartLite_IsReceiveEmpty(XPAR_AXI_UARTLITE_0_BASEADDR)) {
		/* read a character */
		c = XUartLite_RecvByte(XPAR_AXI_UARTLITE_0_BASEADDR);
		if (c == 'p') {

			if (*(select_reg) == 0) {
				*(audio_reg) = (*(audio_reg)) & 6;
				*(select_reg) = 1;
				paused = 1;
			} else if (*(select_reg) == 1){
				*(audio_reg) = (*(audio_reg)) | 1;
				*(select_reg) = 0;
				paused = 0;
			}
		}

		if (c == 'r') {
			currenttime = 99;
            timer = 0;
			*(score_reg) = 0;
			*(score_2_reg) = 0;
			*(select_reg) = 3;
			paused = 0;
			start = 0;

		}

		if (c == 'm') {
	    	*(audio_reg) = (*(audio_reg)) ^ 1;
		}

	    if (!start && c == '1') {
	    	currenttime = 99;
	    	*(mode) = 0;
	    	timer = 0;
	    	*(select_reg) = 0;
	    	start = 1;
	    }

	    if (!start && c == '2') {
	    	currenttime = 99;
	    	*(mode) = 1;
	    	timer = 0;
	    	*(select_reg) = 0;
	    	start = 1;
	    }

	    if (c == 'd') {
			c = XUartLite_RecvByte(XPAR_AXI_UARTLITE_0_BASEADDR);
			switch (c){
			case '1':
				diff = 1;
				period = 50;
				break;
			case '2':
				diff = 2;
				period = 40;
				break;
			case '3':
				diff = 3;
				period = 30;
				break;
			case '4':
				diff = 4;
				period = 20;
				break;
			case '5':
				diff = 5;
				period = 10;
				break;
			default:
				diff = 1;
				period = 50;
			}

		}
	}
}

void navigator() {
	while (1) {
		// loop waiting for interrupt
	}
}

int main()
{

    init_platform();

    /* Enable MicroBlaze exception */
    microblaze_enable_interrupts();

	/* Connect uart interrupt handler that will be called when an interrupt
	* for the uart occurs*/
	XIntc_RegisterHandler(XPAR_INTC_0_BASEADDR,XPAR_MICROBLAZE_0_AXI_INTC_AXI_UARTLITE_0_INTERRUPT_INTR,(XInterruptHandler)uart_int_handler,(void *)XPAR_AXI_UARTLITE_0_BASEADDR);

	/* Start the interrupt controller */
	XIntc_MasterEnable(XPAR_INTC_0_BASEADDR);

	/* Enable uart interrupt in the interrupt controller */
	XIntc_EnableIntr(XPAR_INTC_0_BASEADDR, XPAR_AXI_UARTLITE_0_INTERRUPT_MASK);

	/* Enable Uartlite interrupt */
	XUartLite_EnableIntr(XPAR_AXI_UARTLITE_0_BASEADDR);

	srand(21345);

	*(score_reg) = 0;
	*(score_2_reg) = 0;
	*(select_reg) = 3;

	// start of game software
    int x_tar = rand() % 300;
    int y_tar = rand() % 300;
    // 0 = stationary
    // 1 = east
    // 2 = se, etc. clockwise
    // 8 = ne
    int periodcounter = 0;
    int direction = rand() % 8 + 1;
	//xil_printf("targx: %d, targy: %d\n\r", x_tar, y_tar);
    int speed = 0;
    int score = 0;
    int px = 0;
    int py = 0;
    int xfar = 0;
    int yfar = 0;
    int audio = 1;
    int atrigger = 0;
    int audiotimer = 0;
    *(score_reg) = score;
    *(x_target_reg) = x_tar;
    *(y_target_reg) = y_tar;
    *(audio_reg) = audio;
    xil_printf("Pause/Unpause : p \n");
    xil_printf("Mute/Unmute : m \n");
    xil_printf("restart : r \n");
    xil_printf("Difficulty : 'd' + '(# from 1 to 5)' eg:d1 for lowest difficulty d5 for highest difficulty \n");
    xil_printf("Gamemode : 1 for singleplayer 2 for multiplayer \n");
    while (1) {
    	// spin while paused
    	while (paused) {}

    	// delay loop
    	for (int i = 0; i < 100000; i++){}

    	// increment gamelength
    	if ((timer % 100) == 0 ) {
    	    *(timer_reg) = currenttime;
    	    if(currenttime != 0){
    	    currenttime -= 1;
    	    }
    	}

    	if (timer >= gamelength) {
    		paused = 1;
    		*(select_reg) = 2;

    	}

    	timer += 1;

    	if ((periodcounter % period) == 0 ){
    	    speed = (rand() % 3) + diff;
        }


    	if (periodcounter == period * 2){
    	  	direction = rand() % 8 + 1;
    	    periodcounter = 0;
    	}

    	// moving target
    	switch (direction) {
    	case 0:
    		x_tar = x_tar;
    		y_tar = y_tar;
    		break;
    	case 1:
    		if (x_tar >= 300){
    			x_tar = 0;
    		} else {
    			x_tar = x_tar + speed;
    		}
    		break;
    	case 2:
			if (x_tar >= 300){
				x_tar = 0;
			} else {
				x_tar = x_tar + speed;
			}
			if (y_tar >= 300) {
				y_tar = 0;
			} else {
				y_tar = y_tar + speed;
			}
			break;
    	case 3:
			if (y_tar >= 300) {
				y_tar = 0;
			} else {
				y_tar = y_tar + speed;
			}
			break;
    	case 4:
			if (x_tar <= 0){
				x_tar = 300;
			} else {
				x_tar = x_tar - speed;
			}
			if (y_tar >= 300) {
				y_tar = 0;
			} else {
				y_tar = y_tar + speed;
			}
			break;
    	case 5:
			if (x_tar <= 0){
				x_tar = 300;
			} else {
				x_tar = x_tar - speed;
			}
			break;
    	case 6:
			if (x_tar <= 0){
				x_tar = 300;
			} else {
				x_tar = x_tar - speed;
			}
			if (y_tar <= 0) {
				y_tar = 300;
			} else {
				y_tar = y_tar - speed;
			}
			break;
    	case 7:
			if (y_tar <= 0) {
				y_tar = 300;
			} else {
				y_tar = y_tar - speed;
			}
			break;
    	case 8:
			if (x_tar >= 300){
				x_tar = 0;
			} else {
				x_tar = x_tar + speed;
			}
			if (y_tar <= 0) {
				y_tar = 300;
			} else {
				y_tar = y_tar - speed;
			}
			break;
    	}

    	*(x_target_reg) = x_tar;
    	*(y_target_reg) = y_tar;

    	// detect laser
    	int x_det = *(x_reg);
    	int y_det = *(y_reg);
    	int x_det_green = *(x_reg_green);
    	int y_det_green = *(y_reg_green);

    	if (atrigger == 1){
            audiotimer++;
            if(audiotimer == 50){
            	*(audio_reg) = (*(audio_reg)) ^ 2;
            	 //audiotimer = 0;
            	 atrigger = 0;
            }
    	}
    	//xil_printf("x: %d, y: %d\n\r", x_det, y_det);

    	// 1player
    	if (*mode == 0){
			if ((abs(x_det - x_tar) <= 15 && abs(y_det - y_tar) <= 15) || (abs(x_det_green - x_tar) <= 15 && abs(y_det_green - y_tar) <= 15) && ((x_det != 0 && y_det != 0) || (x_det_green != 0 && y_det_green != 0))) {
				score = *(score_reg);
				audiotimer = 0;
				if((*(audio_reg) & 2) != 2 ){
				*(audio_reg) = (*(audio_reg)) ^ 2;
				}
				atrigger = 1;
				//xil_printf("targx: %d, targy: %d, score: %d\n\r", x_tar, y_tar, score);
				if(x_det != 0 || y_det != 0){
				score ++;
				}
				*(score_reg) = score;
				px = x_tar;
				py = y_tar;
				xfar = 0;
				yfar = 0;
				while (!xfar){
				x_tar = rand() % 280 + 10;
				if (abs(x_tar - px) >= 50){
					xfar = 1;
				}
				}

				while (!yfar){
				y_tar = rand() % 280 + 10;
				if (abs(y_tar - py) >= 50){
					yfar = 1;
				}
				}
				*(x_target_reg) = x_tar;
				*(y_target_reg) = y_tar;
				direction = rand() % 8 + 1;
				periodcounter = 0;
			}
			else{
				periodcounter++;
			}
    	}

    	// 2player
    	else if (*mode == 1){
			if ((abs(x_det - x_tar) <= 10 && abs(y_det - y_tar) <= 10)) {
				score = *(score_reg);
				audiotimer = 0;
				if((*(audio_reg) & 2) != 2 ){
				*(audio_reg) = (*(audio_reg)) ^ 2;
				}
				atrigger = 1;
				//xil_printf("targx: %d, targy: %d, score: %d\n\r", x_tar, y_tar, score);
				if(x_det != 0 || y_det != 0){
						score ++;
				}
				*(score_reg) = score;
				px = x_tar;
				py = y_tar;
				xfar = 0;
				yfar = 0;
				while (!xfar){
				x_tar = rand() % 280 + 10;
				if (abs(x_tar - px) >= 50){
					xfar = 1;
				}
				}

				while (!yfar){
				y_tar = rand() % 280 + 10;
				if (abs(y_tar - py) >= 50){
					yfar = 1;
				}
				}
				*(x_target_reg) = x_tar;
				*(y_target_reg) = y_tar;
				direction = rand() % 8 + 1;
				periodcounter = 0;
			}
			else if ((abs(x_det_green - x_tar) <= 20 && abs(y_det_green - y_tar) <= 20) ) {
				score = *(score_2_reg);
				audiotimer = 0;
				if((*(audio_reg) & 2) != 2 ){
				*(audio_reg) = (*(audio_reg)) ^ 2;
				}
				atrigger = 1;
				//xil_printf("targx: %d, targy: %d, score: %d\n\r", x_tar, y_tar, score);
				score ++;
				*(score_2_reg) = score;
				px = x_tar;
				py = y_tar;
				xfar = 0;
				yfar = 0;
				while (!xfar){
				x_tar = rand() % 280 + 10;
				if (abs(x_tar - px) >= 50){
					xfar = 1;
				}
				}

				while (!yfar){
				y_tar = rand() % 280 + 10;
				if (abs(y_tar - py) >= 50){
					yfar = 1;
				}
				}
				*(x_target_reg) = x_tar;
				*(y_target_reg) = y_tar;
				direction = rand() % 8 + 1;
				periodcounter = 0;
			} else {
				periodcounter++;
			}
		}
    }

    cleanup_platform();
    return 0;
}
