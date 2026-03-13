// exampleApp.cpp
// update 202603
#include <iostream>
#include <unistd.h>
#include "jetsonGPIO.h"

using namespace std;

static int initGpio(jetsonXavierGPIONumber gpio)
{
    if (gpioExport(gpio) < 0) {
        cerr << "Failed to export GPIO " << gpio << endl;
        return -1;
    }

    if (gpioSetDirection(gpio, outputPin) < 0) {
        cerr << "Failed to set direction for GPIO " << gpio << endl;
        gpioUnexport(gpio);
        return -1;
    }

    return 0;
}

static void cleanupGpio(jetsonXavierGPIONumber gpio)
{
    gpioSetValue(gpio, off);
    gpioUnexport(gpio);
}

int main()
{
    constexpr jetsonXavierGPIONumber redLED = gpio417;
    constexpr useconds_t blinkDelayUs = 2000000;   // 2 seconds
    constexpr int blinkCount = 5;

    cout << "Testing the GPIO Pins" << endl;

    if (initGpio(redLED) < 0) {
        return EXIT_FAILURE;
    }

    for (int i = 0; i < blinkCount; ++i) {
        cout << "Blink " << (i + 1) << "/" << blinkCount << ": LED on" << endl;
        if (gpioSetValue(redLED, on) < 0) {
            cerr << "Failed to set LED on" << endl;
            cleanupGpio(redLED);
            return EXIT_FAILURE;
        }

        usleep(blinkDelayUs);

        cout << "Blink " << (i + 1) << "/" << blinkCount << ": LED off" << endl;
        if (gpioSetValue(redLED, off) < 0) {
            cerr << "Failed to set LED off" << endl;
            cleanupGpio(redLED);
            return EXIT_FAILURE;
        }

        usleep(blinkDelayUs);
    }

    cleanupGpio(redLED);
    cout << "GPIO example finished." << endl;

    return EXIT_SUCCESS;
}
