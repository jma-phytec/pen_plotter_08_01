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

volatile uint32_t gUserSharedMem __attribute__((aligned(128), section(".bss.user_shared_mem")));

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
    if(isMove)
        DebugP_log("Motor positioning %d cur_pos %f next_pos %f\r\n", Motor->positioning, Motor->cur_pos, Motor->next_pos);
    //DebugP_log("gUserSharedMem 0x%x\r\n", gUserSharedMem);
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
        if(isMove)
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
    {
        Motor->cur_pos = 0;
#ifdef MOTORZ
        Motor->next_pos = 0;
#endif
    }
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
    Motor->step_base_addr = STEP_X_BASE_ADDR;
    Motor->step_pin = STEP_X_PIN;
    Motor->step_dir = STEP_X_DIR;
    Motor->dir_base_addr = DIR_X_BASE_ADDR;
    Motor->dir_pin = DIR_X_PIN;
    Motor->dir_dir = DIR_X_DIR;
#endif

#ifdef MOTORY
    Motor->step_base_addr = STEP_Y_BASE_ADDR;
    Motor->step_pin = STEP_Y_PIN;
    Motor->step_dir = STEP_Y_DIR;
    Motor->dir_base_addr = DIR_Y_BASE_ADDR;
    Motor->dir_pin = DIR_Y_PIN;
    Motor->dir_dir = DIR_Y_DIR;
#endif

#ifdef MOTORZ
    Motor->step_base_addr = STEP_Z_BASE_ADDR;
    Motor->step_pin = STEP_Z_PIN;
    Motor->step_dir = STEP_Z_DIR;
    Motor->dir_base_addr = DIR_Z_BASE_ADDR;
    Motor->dir_pin = DIR_Z_PIN;
    Motor->dir_dir = DIR_Z_DIR;
#endif

    gpio_motor_control_setSpeed(Motor, 6000);   // 6000mm per min = 100mm per sec

    if(isMove)
    {
#ifdef MOTORX
        // Enable 8255 EN pin
        uint32_t    gpioBaseAddr, pinNum;
        /* Get address after translation translate */
        gpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(EN_X_BASE_ADDR);
        pinNum       = EN_X_PIN;
        GPIO_setDirMode(gpioBaseAddr, EN_X_PIN, EN_X_DIR);
        GPIO_pinWriteLow(gpioBaseAddr, pinNum);

        gpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(STEP_X_BASE_ADDR);
        pinNum       = STEP_X_PIN;
        GPIO_setDirMode(gpioBaseAddr, STEP_X_PIN, STEP_X_DIR);

        gpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(DIR_X_BASE_ADDR);
        pinNum       = DIR_X_PIN;
        GPIO_setDirMode(gpioBaseAddr, DIR_X_PIN, DIR_X_DIR);

#endif
#ifdef MOTORY
        // Enable 8255 EN pin
        uint32_t    gpioBaseAddr, pinNum;
        /* Get address after translation translate */
        gpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(EN_Y_BASE_ADDR);
        pinNum       = EN_Y_PIN;
        GPIO_setDirMode(gpioBaseAddr, EN_Y_PIN, EN_Y_DIR);
        GPIO_pinWriteLow(gpioBaseAddr, pinNum);

        gpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(STEP_Y_BASE_ADDR);
        pinNum       = STEP_Y_PIN;
        GPIO_setDirMode(gpioBaseAddr, STEP_Y_PIN, STEP_Y_DIR);

        gpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(DIR_Y_BASE_ADDR);
        pinNum       = DIR_Y_PIN;
        GPIO_setDirMode(gpioBaseAddr, DIR_Y_PIN, DIR_Y_DIR);

#endif
#ifdef MOTORZ
        // Enable 8255 EN pin
        uint32_t    gpioBaseAddr, pinNum;
        /* Get address after translation translate */
        gpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(EN_Z_BASE_ADDR);
        pinNum       = EN_Z_PIN;
        GPIO_setDirMode(gpioBaseAddr, EN_Z_PIN, EN_Z_DIR);
        GPIO_pinWriteLow(gpioBaseAddr, pinNum);

        gpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(STEP_Z_BASE_ADDR);
        pinNum       = STEP_Z_PIN;
        GPIO_setDirMode(gpioBaseAddr, STEP_Z_PIN, STEP_Z_DIR);

        gpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(DIR_Z_BASE_ADDR);
        pinNum       = DIR_Z_PIN;
        GPIO_setDirMode(gpioBaseAddr, DIR_Z_PIN, DIR_Z_DIR);

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
            int bHomeSwitchX, bHomeSwitchY, bHomeSwitchZ;
            if(gUserSharedMem & 0x8)
                bHomeSwitchX = 1;
            else
                bHomeSwitchX = 0;
            if(gUserSharedMem & 0x10)
                bHomeSwitchY = 1;
            else
                bHomeSwitchY = 0;
            if(gUserSharedMem & 0x20)
                bHomeSwitchZ = 1;
            else
                bHomeSwitchZ = 0;
#ifdef MOTORX
//            if((bHomeSwitch==0) || ((bHomeSwitch==1) && (Motor->dir==1)))
            if((bHomeSwitchX==0) || ((bHomeSwitchX==1) && (Motor->dir==1)))
#endif
#ifdef MOTORY
//            if((bHomeSwitch==0) || ((bHomeSwitch==1) && (Motor->dir==0)))
              if((bHomeSwitchY==0) || ((bHomeSwitchY==1) && (Motor->dir==0)))
#endif
#ifdef MOTORZ
//            if((bHomeSwitch==0) || ((bHomeSwitch==1) && (Motor->dir==0)))
            if((bHomeSwitchZ==0) || ((bHomeSwitchZ==1) && (Motor->dir==0)))
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
#ifdef MOTORX
    if(Motor->dir==0)
        GPIO_pinWriteLow(gpioBaseAddr, pinNum);
    else
        GPIO_pinWriteHigh(gpioBaseAddr, pinNum);
#endif
#ifdef MOTORY
    if(Motor->dir==0)
        GPIO_pinWriteLow(gpioBaseAddr, pinNum);
    else
        GPIO_pinWriteHigh(gpioBaseAddr, pinNum);
#endif
#ifdef MOTORZ
    if(Motor->dir==0)
        GPIO_pinWriteLow(gpioBaseAddr, pinNum);
    else
        GPIO_pinWriteHigh(gpioBaseAddr, pinNum);
#endif

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
