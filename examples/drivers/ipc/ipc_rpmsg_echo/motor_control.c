/*
 *  Motor Control
 *
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <kernel/dpl/HwiP.h>
#include <kernel/dpl/CacheP.h>
#include <kernel/dpl/SemaphoreP.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/HeapP.h>
#include <kernel/dpl/CycleCounterP.h>
#include <drivers/gpio.h>
#include <kernel/dpl/AddrTranslateP.h>
#include <kernel/dpl/DebugP.h>
#include "motor_control.h"
#include <math.h>
#include <drivers/gpio.h>
#include <kernel/dpl/AddrTranslateP.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/ClockP.h>
#include "ti_drivers_config.h"

#define ROUND_LEN 40.5
#define NUM_STEPS_PER_ROTATION 205
#define STEPSIZE 16

char msgBuf[128];
MotorMod MotorX, MotorY, MotorZ;

float CalculateMotorLoop(MotorMod *Motor, float Displayment)
{
    float StepsRequired = 0;
    // 200 steps = 1 rotation = 4.04cm = 40.4mm
    StepsRequired = (Displayment / ROUND_LEN) * NUM_STEPS_PER_ROTATION * STEPSIZE;
#ifdef MOTORX
    StepsRequired = StepsRequired * 0.5;
#endif
#ifdef MOTORY
    StepsRequired = StepsRequired * 0.5;
#endif
#ifdef MOTORZ
    StepsRequired = StepsRequired * 3;
#endif

#ifdef MYDEBUG
    DebugP_log("CalculateMotorLoop: Displayment %f StepsRequired %f\r\n", Displayment, StepsRequired);
#endif
    return StepsRequired;
}

void gpio_motor_move(MotorMod *Motor, Bool isNegative, Bool isMove, float Ratio)
{
    float Displacement;
    float LoopRequired;

#ifdef MYDEBUG
    DebugP_log("Motor positioning %d cur_pos %f next_pos %f\r\n", Motor->positioning, Motor->cur_pos, Motor->next_pos);
#endif

    if(Motor->positioning==90) // Absolute
    {
        Displacement = Motor->next_pos - Motor->cur_pos;
        if(Motor->next_pos > Motor->cur_pos)
            Motor->dir = 1;
        else
            Motor->dir = 0;

        gpio_motor_control_dir_main(Motor);
        Motor->cur_pos = Motor->next_pos;
    }
    else    // Relative
    {
        if(isNegative==0)
        {
            Motor->dir = 1;
	    Displacement = Motor->next_pos - Motor->cur_pos;
            Motor->cur_pos = Motor->next_pos;
        }
        else
        {
            Motor->dir = 0;
	    Displacement = Motor->cur_pos - Motor->next_pos;
            Motor->cur_pos = Motor->next_pos;
        }
        gpio_motor_control_dir_main(Motor);
    }
    if(isMove)
    {
        LoopRequired = CalculateMotorLoop(Motor, fabs(Displacement));
        gpio_motor_control_step_main(Motor, LoopRequired, Ratio);
#ifdef MYDEBUG
        DebugP_log("isNegative %d Motor->dir %d Motor->cur_pos %f LoopRequired %f \r\n", isNegative, Motor->dir, Motor->cur_pos, LoopRequired);
#endif
    }
    return;
}


void gpio_motor_control_ioctl(MotorMod *Motor, uint8_t cmd, uint32_t val)
{
    if(cmd == UPDATE_POSITIONING)
        Motor->positioning = val;
    else if(cmd == UPDATE_DIR)
        Motor->dir = val;
    else if(cmd == UPDATE_UNIT)
    {
        Motor->unit = val;
        gpio_motor_control_setSpeed(Motor, Motor->speed);
    }
    else if(cmd == UPDATE_MOVING)
        Motor->moving = val;
    else if(cmd == UPDATE_HOME)
        Motor->cur_pos = 0;
    return;
}

void gpio_motor_control_setCurPos(MotorMod *Motor, float val)
{
    Motor->cur_pos = val;
    return;
}

void gpio_motor_control_setNextPos(MotorMod *Motor, float val, int isNegative)
{
    if(Motor->positioning==90)	// Absolute
	    Motor->next_pos = val;
    else
    {
	Motor->next_pos = Motor->next_pos + val;
    }
    return;
}

#define FACTOR 10

void gpio_motor_control_setSpeed(MotorMod *Motor, float val)
{
    Motor->speed = val/60;  // per min
    if(Motor->unit==20) // inches
    {
        Motor->pulse_width = (Motor->speed * 25.4) * FACTOR;  // TO DO
    }
    else    // millimeters
    {
        Motor->pulse_width = Motor->speed * FACTOR;  // TO DO
    }
    return;
}

void gpio_motor_control_init(MotorMod *Motor, Bool isMove)
{
    Motor->cur_pos = 0;
    Motor->next_pos = 0;
    Motor->positioning = 90;
    Motor->dir = 0;
    Motor->unit = 21;
    Motor->moving = 0;
    Motor->isActive = TRUE;

#ifdef MOTORX
    Motor->step_base_addr = P7_BASE_ADDR;
    Motor->step_pin = P7_PIN;
    Motor->step_dir = P7_DIR;
    Motor->dir_base_addr = P9_BASE_ADDR;
    Motor->dir_pin = P9_PIN;
    Motor->dir_dir = P9_DIR;
#endif

#ifdef MOTORY
    Motor->step_base_addr = P11_BASE_ADDR;
    Motor->step_pin = P11_PIN;
    Motor->step_dir = P11_DIR;
    Motor->dir_base_addr = P17_BASE_ADDR;
    Motor->dir_pin = P17_PIN;
    Motor->dir_dir = P17_DIR;
#endif

#ifdef MOTORZ
    Motor->step_base_addr = P16_BASE_ADDR;
    Motor->step_pin = P16_PIN;
    Motor->step_dir = P16_DIR;
    Motor->dir_base_addr = P18_BASE_ADDR;
    Motor->dir_pin = P18_PIN;
    Motor->dir_dir = P18_DIR;
#endif

    gpio_motor_control_setSpeed(Motor, 6000);   // 6000mm per min = 100mm per sec

    if(isMove)
    {
#if 0
        // Enable 8255 EN pin
        uint32_t    gpioBaseAddr, pinNum;

        /* Get address after translation translate */
        gpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(GPIO_EN_BASE_ADDR);
        pinNum       = GPIO_EN_PIN;

        GPIO_setDirMode(gpioBaseAddr, GPIO_EN_PIN, GPIO_EN_DIR);
        GPIO_pinWriteLow(gpioBaseAddr, pinNum);

        // Enable LED
        GPIO_setDirMode(gpioBaseAddr, RED_GPIO_PIN, RED_GPIO_DIR);
        GPIO_setDirMode(gpioBaseAddr, GREEN_GPIO_PIN, GREEN_GPIO_DIR);
        GPIO_setDirMode(gpioBaseAddr, BLUE_GPIO_PIN, BLUE_GPIO_DIR);

#ifdef MOTORX
        GPIO_pinWriteHigh(gpioBaseAddr, RED_GPIO_PIN);
        GPIO_pinWriteLow(gpioBaseAddr, GREEN_GPIO_PIN);
        GPIO_pinWriteLow(gpioBaseAddr, BLUE_GPIO_PIN);
#endif

#ifdef MOTORY
        GPIO_pinWriteLow(gpioBaseAddr, RED_GPIO_PIN);
        GPIO_pinWriteHigh(gpioBaseAddr, GREEN_GPIO_PIN);
        GPIO_pinWriteLow(gpioBaseAddr, BLUE_GPIO_PIN);
#endif

#ifdef MOTORZ
        GPIO_pinWriteLow(gpioBaseAddr, RED_GPIO_PIN);
        GPIO_pinWriteLow(gpioBaseAddr, GREEN_GPIO_PIN);
        GPIO_pinWriteHigh(gpioBaseAddr, BLUE_GPIO_PIN);
#endif
#endif
    }

    return;
}

void gpio_motor_control_step_main(MotorMod *Motor, float StepsRequired, float Ratio)
{
    uint32_t    loopcnt = (uint32_t)(StepsRequired+0.5);
    uint32_t    gpioBaseAddr, pinNum;
    int32_t     width;

    /* Get address after translation translate */
    gpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(Motor->step_base_addr);
    pinNum       = Motor->step_pin;

    width = 300 * Ratio;
#ifdef MYDEBUG
    DebugP_log("gpio_motor_control_step_main loopcnt %d width %d\r\n", loopcnt, width);
#endif

    GPIO_setDirMode(gpioBaseAddr, pinNum, Motor->step_dir);
    while(loopcnt>0)
    {
        if(Motor->isActive)
        {
#ifdef MOTORX
//            if((bHomeSwitch==0) || ((bHomeSwitch==1) && (Motor->dir==1)))
#endif
#ifdef MOTORY
//            if((bHomeSwitch==0) || ((bHomeSwitch==1) && (Motor->dir==0)))
#endif
#ifdef MOTORZ
//            if((bHomeSwitch==0) || ((bHomeSwitch==1) && (Motor->dir==0)))
#endif
            {
                GPIO_pinWriteHigh(gpioBaseAddr, pinNum);
                ClockP_usleep(width);
                GPIO_pinWriteLow(gpioBaseAddr, pinNum);

                ClockP_usleep(width);
            }
        }
        loopcnt--;
    }

    //DebugP_log("GPIO1 STEP pin toggle finishs ...\r\n");

    return;
}

void gpio_motor_control_dir_main(MotorMod *Motor)
{
    uint32_t    gpioBaseAddr, pinNum;

    //DebugP_log("GPIO DIR pin toggle starts ...\r\n");

    /* Get address after translation translate */
    gpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(Motor->dir_base_addr);
    pinNum       = Motor->dir_pin;

    GPIO_setDirMode(gpioBaseAddr, pinNum, Motor->dir_dir);
    if(Motor->dir==0)
        GPIO_pinWriteLow(gpioBaseAddr, pinNum);
    else
        GPIO_pinWriteHigh(gpioBaseAddr, pinNum);

    ClockP_usleep(100);

    return;
}


int motor_demo_init(void)
{
#ifdef MOTORX
    gpio_motor_control_init(&MotorX, TRUE);
#else
    gpio_motor_control_init(&MotorX, FALSE);
#endif
#ifdef MOTORY
    gpio_motor_control_init(&MotorY, TRUE);
#else
    gpio_motor_control_init(&MotorY, FALSE);
#endif
#ifdef MOTORZ
    gpio_motor_control_init(&MotorZ, TRUE);
#else
    gpio_motor_control_init(&MotorZ, FALSE);
#endif

    return 0;
}
