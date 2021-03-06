// C standard include(s):
#include <math.h>
#include <stdio.h>

// Custom user APIs needed for hardware access specific to this board:
#include "cpu.h"
#include "pwm_hal.h"
#include "sensor_hal.h"
#include "general.h"
#include "uart.h"
#include "frontQueue.h"
#include "encoder_hal.h"
#include "board_led.h"

// // Custom user APIs needed for generic algorithmic libraries that are hardware-independent:
#include "foo.h"

void parallel_park() {
    // Forward
    while (encoders_distances.left < 10)
        motors_control(NORMAL, 0.0f, 0.0f, NORMAL);

    // Reset encoders
    encoders_distances.right = 0;
    encoders_distances.left = 0;

    // Right turn
    while (encoders_distances.left < 20)
        motors_control(0.3f, 0.0f, 0.0f, 1.0f);

    // Stop and reset encoders
    motors_control(0.0f,0.0f, 0.0f,0.0f);
    cpu_sw_delay(100U);
    encoders_distances.right = 0;
    encoders_distances.left = 0;

    // Backup
    while (encoders_distances.left < 10)
        motors_control(0.0f, NORMAL, NORMAL, 0.0f);

    // Reset encoders
    encoders_distances.right = 0;
    encoders_distances.left = 0;

    // Left back turn
	while (encoders_distances.right < 20)
		motors_control(0.0f, 1.0f, 0.3f, 0.0f);

    // Stop and reset encoders
    motors_control(0.0f,0.0f, 0.0f,0.0f);
    cpu_sw_delay(100U);
    encoders_distances.right = 0;
    encoders_distances.left = 0;

    // Right forward turn
    while (encoders_distances.left < 7)
        motors_control(0.6f, 0.0f, 0.0f, 1.0f);

    // Stop
    motors_control(0.0f,0.0f, 0.0f,0.0f);
}

void perpendicular_park() {
	// Forward
    while (encoders_distances.right < 56|| encoders_distances.left < 56)
        motors_control(NORMAL, 0.0f, 0.0f, NORMAL1);

	// Stop and reset encoders
    motors_control(0.0f,0.0f, 0.0f,0.0f);
    cpu_sw_delay(100U);
    encoders_distances.right = 0;
    encoders_distances.left = 0;

	// Backup
    while (encoders_distances.right < 19|| encoders_distances.left < 19)
        motors_control(0.0f, 1.0f, 1.0f, 0.0f);

    // Reset encoders
    encoders_distances.right = 0;
    encoders_distances.left = 0;

	// Reverse right turn
    while (encoders_distances.left < 90)
        motors_control(0.0f, 0.3f, 1.0f, 0.0f);

    // Stop and reset encoders
    motors_control(0.0f, NORMAL, NORMAL, 0.0f);
	cpu_sw_delay(20U);
    encoders_distances.right = 0;
    encoders_distances.left = 0;

    // Safety
    distances.right = 30.0f;
}

void init_sensors_values() {
    right_triggered = 0;
    front_triggered = 0;
    left_triggered = 0;

    distances.right = 0.0f;
    distances.left = 0.0f;

    treshDist.left = 10.0f;
    treshDist.right = 10.0f;
}

void init_pins() {
    echosPins.right = GPIO_PIN_11;
    echosPins.front = GPIO_PIN_6;
    echosPins.left = GPIO_PIN_9;

    triggerPins.right = GPIO_PIN_10;
    triggerPins.front = GPIO_PIN_0;
    triggerPins.left = GPIO_PIN_0;

    encodersPins.right = GPIO_PIN_3;
    encodersPins.left = GPIO_PIN_4;
}

int main()
{
    /*
     Initialize the PLL, clock tree to all peripherals, flash and Systick 1 ms time reference:
     */
    cpu_init();

    /* Debug uart port initialization and test */
    uart_debug_init();
    setvbuf(stdin,NULL,_IONBF,0);   // Sets stdin in unbuffered mode (normal for usart com)
    setvbuf(stdout,NULL,_IONBF,0);  // Sets stdin in unbuffered mode (normal for usart com)
    //printf("Hello World!!\r\n");

    /* Initialize motor state and parameters */
    int turned_left = 0;
    int firstTurn = 0;
    int count = 0;
    motorState = STOP;
    searching = 1;
    delay = 0;
    found = 0;
	NORMAL = 0.6f;
	NORMAL1 = 0.8f;

    /* Initialize pwm to control motors */
    init_pwm();

    /* Initialize pin values */
    init_pins();

    /* Initialize sensors */
    init_sensors_values();
    init_triggers();
    init_echos();
    init_timers();
    init_queue(); // Initialize sensor value averaging

    /* Initialize encoders and  */
    init_encoders();
    encoders_distances.left = 0;
    encoders_distances.right = 0;

    /* Initialize leds for debugging */
    board_led_init();

    int i = 0;

    while(1)
    {
        /*
         Carry out a simple unit test of foo() declared in foo.h:
         */
        if(TEST_FOO(i, i+1) < 0)
        {
            /*
             If the above fails there is either a hardware, code or other undefined error.
             Now we're in an undefined state regarding processor core behavior...
             */
            while(1); // We probably have had a radiation hit or a major malfunction on the ALU of the processor...
        }
        else
        {
		    if (motorState == STRAIGHT) {
               	// Turn on appropriate leds to identify straight state
				board_led_on(LED6);
                board_led_off(LED9);
                board_led_off(LED8);

				// Function call to make car go staright
                adjust();

				// If still searching for the parking spot
                if (searching == 1) {
				// If right after a left turn add a delay before using sensors again
                    if (delay == 1) {
                        if (encoders_distances.right >= 5) {
                            delay = 0;
                            turned_left = 1;
/* FOR EMERGENCY USE
							motors_control(0.0f, 0.0f, 0.0f, 0.0f);
							cpu_sw_delay(20U);
*/
                        }
                    } else {
						// If very bg=ig gap, then not parking spot, make a right turn
                        if (distances.right > 70.0f) {
							// reset encoders values
                            if (firstTurn == 0) {
                                encoders_distances.right = 0;
                                firstTurn = 1;
                            }

							// If after a left turn, no need to add additional delay
                            if (turned_left == 1) {
                                motorState = RIGHTD;
                                searching = 0;
                                encoders_distances.left = 0;
                                encoders_distances.right = 0;
                            } else {
								// Add delay to avoid wall on the right side
                                if (encoders_distances.right >= 5) {
                                    motorState = RIGHTD;
                                    searching = 0;
                                    encoders_distances.left = 0;
                                    encoders_distances.right = 0; 
                                }
                            }
                        }
						// If right sensors senses a distance between 25 and 70, spot found
                        else if (distances.right > 25.0f) {
                            searching = 0;
                        }
                    }
                } 
                else {
					// Add delay before checking right sensor
                    if (encoders_distances.left >= 30) {
						// Gap found
                        if (found == 0 && distances.right > 25.0f)
                            found = 1;
						// After the gap was seen, if the car is next to the parked car go to perpendicular park routine
                        else if (found == 1 && distances.right < 25.0f) {
                            encoders_distances.right = 0;
                            encoders_distances.left = 0;
                            perpendicular_park();
							// Make sure that the system will not go from STOP to STRAIGHT state
                            count = FRONTQUEUEMAX + 1;
                            motorState = BACK;
                        }
                    }
                }
            }
            else if (motorState == STOP) {
                motors_control(0.0f,0.0f,0.0f,0.0f);

               	// Turn on appropriate leds to identify stop state
                board_led_off(LED6);
                board_led_on(LED9);
                board_led_off(LED8);
				
				// If buffer initialized with values start
                if (count == FRONTQUEUEMAX)
                    motorState = STRAIGHT;
            }
            else if (motorState == LEFTD) {

               	// Turn on appropriate leds to identify left state
                board_led_off(LED6);
                board_led_off(LED9);
                board_led_on(LED8);

				// Keep turning left until 90 degree turn done (encoders) and car more or less parallel to the wall (sensors)
                if (encoders_distances.right >= 62 && (distances.right <= treshDist.right + 5.0f && distances.right >= treshDist.right - 5.0f)) {
                    motorState = STRAIGHT;
					// Add a delay when in straight mode to give the sensors a chance to read good values again
                    delay = 1;
                    distances.right = 0;
                    encoders_distances.right = 0;
                    encoders_distances.left = 0;
                }

                motors_control(NORMAL, 0.0f, 0.0f, 0.0f);
            }
            else if (motorState == RIGHTD) {

               	// Turn on appropriate leds to identify right state
                board_led_on(LED6);
                board_led_off(LED9);
                board_led_on(LED8);

                motors_control(0.3f, 0.0f, 0.0f, NORMAL);

				// Keep turning right until 90 degree turn done (encoders) and car more or less parallel to the wall (sensors)
                if (encoders_distances.left > 130 && (distances.right <= treshDist.right + 7.5f && distances.right >= treshDist.right - 2.5f)) {
                    motorState = STRAIGHT;
                    encoders_distances.left = 0;
                    encoders_distances.right = 0;
                }
            }
            else if (motorState == BACK) {
                motors_control(0.0f, NORMAL, NORMAL, 0.0f);
	
				// Go back until inside the spot
                if (distances.right <= 25.4f) {
                    motors_control(0.0f, 0.0f, 0.0f, 0.0f);
                    cpu_sw_delay(100U);
                    encoders_distances.right = 0;
                    encoders_distances.left = 0;
					// Uncomment to parallel park, otherwise perpendicular park done
                    //parallel_park();
                    motorState = STOP;
                }
            }

            /* Trigger sensors */
            if (front_triggered == 0) {
                if (count < FRONTQUEUEMAX)
                    count++;

                front_triggered = 1;
                trigger_sensor(GPIOD, triggerPins.front);
            }
            if (right_triggered == 0) {
                right_triggered = 1;
                trigger_sensor(GPIOE, triggerPins.right);
            }
        }
    }
    return 0;
}
