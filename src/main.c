import time
from gpiozero import LED

# Configuration
LED_PIN = 17
led = LED(LED_PIN)
LOG_FILE = "occupancy.log"

# Room State
count = 0
max_capacity = 5


def log_event(action):
    timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
    with open(LOG_FILE, "a") as f:
        f.write(f"[{timestamp}] {action} | Count: {count}/{max_capacity}\n")


print("Tracker Active.")
led.off()

while True:
    # Determine room status
    if count >= max_capacity:
        status = "FULL"
    elif count == 0:
        status = "VACANT"
    else:
        status = "AVAILABLE"

    print(f"\nStatus: {status} ({count}/{max_capacity})")
    print("1. Record Entry")
    print("2. Record Exit")
    print("3. Change Max Capacity")
    print("4. Quit")

    choice = input("Selection: ").strip()

    if choice == "1":
        if count >= max_capacity:
            print("Error: Room is already at maximum capacity.")
        else:
            count += 1
            log_event("ENTRY")
            if count == max_capacity:
                led.on()

    elif choice == "2":
        if count <= 0:
            print("Error: Occupancy cannot go below zero.")
        else:
            count -= 1
            led.off()
            log_event("EXIT")

    elif choice == "3":
        try:
            new_max = int(input("Enter new max capacity: "))
            if new_max <= 0:
                print("Error: Capacity must be a positive number.")
            elif new_max < count:
                print("Error: Cannot set capacity lower than current count.")
            else:
                max_capacity = new_max
                log_event(f"CAPACITY_CHANGE_TO_{max_capacity}")
                if count >= max_capacity:
                    led.on()
                else:
                    led.off()
        except ValueError:
            print("Error: Please enter a valid integer.")

    elif choice == "4":
        led.off()
        print("Program closed.")
        break
