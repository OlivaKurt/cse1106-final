#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pigpio.h>

// Configuration
#define LED_PIN 17
#define LOG_FILE "occupancy.log"

// Global Room State
int count = 0;
int max_capacity = 5;

// Function to log events to the file
void log_event(const char *action) {
    FILE *f = fopen(LOG_FILE, "a");
    if (f == NULL) {
        printf("Error: Could not open log file.\n");
        return;
    }

    // Get current system time
    time_t rawtime;
    struct tm *timeinfo;
    char timestamp[20];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Write to file
    fprintf(f, "[%s] %s | Count: %d/%d\n", timestamp, action, count, max_capacity);
    fclose(f);
}

int main() {
    // Initialize pigpio library
    if (gpioInitialise() < 0) {
        printf("pigpio initialization failed.\n");
        return 1;
    }

    // Set GPIO 17 as an output pin
    gpioSetMode(LED_PIN, PI_OUTPUT);
    gpioWrite(LED_PIN, 0); // Ensure LED starts OFF

    printf("Tracker Active.\n");

    char input_buf[10];
    int choice;

    while (1) {
        // Determine and print system status
        printf("\n----------------------------------------\n");
        if (count >= max_capacity) {
            printf("Current Occupancy System Status: ROOM FULL\n");
        } else {
            printf("Current Occupancy System Status: SPACE AVAILABLE\n");
        }
        printf("Count: %d / %d\n", count, max_capacity);
        printf("----------------------------------------\n");

        printf("1. Record Entry\n");
        printf("2. Record Exit\n");
        printf("3. Change Max Capacity\n");
        printf("4. Quit\n");
        printf("Selection: ");

        // Safe user input tracking
        if (!fgets(input_buf, sizeof(input_buf), stdin)) continue;
        choice = atoi(input_buf);

        if (choice == 1) {
            if (count >= max_capacity) {
                printf("Error: The program should not permit a %dth person to enter unless someone has exited.\n", max_capacity + 1);
            } else {
                count++;
                log_event("ENTRY");
                if (count >= max_capacity) {
                    gpioWrite(LED_PIN, 1); // Turn LED ON
                }
            }
        } 
        else if (choice == 2) {
            if (count <= 0) {
                printf("Error: Occupancy cannot go below zero.\n");
            } else {
                count--;
                log_event("EXIT");
                if (count < max_capacity) {
                    gpioWrite(LED_PIN, 0); // Turn LED OFF
                }
            }
        } 
        else if (choice == 3) {
            printf("Enter new max capacity: ");
            if (fgets(input_buf, sizeof(input_buf), stdin)) {
                int new_max = atoi(input_buf);
                if (new_max <= 0) {
                    printf("Error: Capacity must be a positive number.\n");
                } else if (new_max < count) {
                    printf("Error: Cannot set capacity lower than current count.\n");
                } else {
                    max_capacity = new_max;
                    char log_msg[50];
                    sprintf(log_msg, "CAPACITY_CHANGE_TO_%d", max_capacity);
                    log_event(log_msg);

                    // Re-evaluate LED state for the new capacity
                    if (count >= max_capacity) {
                        gpioWrite(LED_PIN, 1);
                    } else {
                        gpioWrite(LED_PIN, 0);
                    }
                }
            }
        } 
        else if (choice == 4) {
            gpioWrite(LED_PIN, 0); // Clean up LED
            printf("Program closed.\n");
            break;
        } 
        else {
            printf("Error: Invalid choice.\n");
        }
    }

    // Terminate pigpio library and release resources
    gpioTerminate();
    return 0;
}
