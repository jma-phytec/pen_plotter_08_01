/*
 *  MotorX Control
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
#include <tiescutils.h>
#include <tiesceoefoe.h>
#include <tiescsoc.h>
#include <applInterface.h>
#include <ecatslv.h>
#include <drivers/gpio.h>
#include <kernel/dpl/AddrTranslateP.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/ClockP.h>
#include <ti_drivers_open_close.h>
#include <ti_board_open_close.h>

#define ROUND_LEN 40.5
#define NUM_STEPS_PER_ROTATION 205
#define STEPSIZE 16

char msgBuf[128];

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

    Motor->step_base_addr = GPIO_MOTOR_STEP_BASE_ADDR;
    Motor->step_pin = GPIO_MOTOR_STEP_PIN;
    Motor->step_dir = GPIO_MOTOR_STEP_DIR;
    Motor->dir_base_addr = GPIO_MOTOR_DIR_BASE_ADDR;
    Motor->dir_pin = GPIO_MOTOR_DIR_PIN;
    Motor->dir_dir = GPIO_MOTOR_DIR_DIR;

    gpio_motor_control_setSpeed(Motor, 6000);   // 6000mm per min = 100mm per sec

    if(isMove)
    {
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
            if((bHomeSwitch==0) || ((bHomeSwitch==1) && (Motor->dir==1)))
#endif
#ifdef MOTORY
            if((bHomeSwitch==0) || ((bHomeSwitch==1) && (Motor->dir==0)))
#endif
#ifdef MOTORZ
            if((bHomeSwitch==0) || ((bHomeSwitch==1) && (Motor->dir==0)))
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

int update_gcode_cmdbuf(uint8_t TmpCount, uint8_t TmpCmd, uint16_t TmpMotorData)
{
    static int idx = 0;

    if(idx==0)
        memset(msgBuf, 0, 128-1);

    msgBuf[idx] = (char)TmpCount;
    msgBuf[idx+1] = (char)TmpCmd;
    msgBuf[idx+2] = (char)(TmpMotorData>>8);
    msgBuf[idx+3] = (char)(TmpMotorData & 0xff);


    if((TmpCount==0) || (TmpCmd==0) || (msgBuf[idx+2]==0) || (msgBuf[idx+3]==0))
    {
#ifdef MYDEBUG
        DebugP_log("msgBug %s\r\n", msgBuf);
#endif
        idx = 0;
        return 1;
    }
    else
        idx = idx + 4;

    return 0;
}

#define SMOOTH 2

int print_GCode(uint8_t TmpCount, uint8_t TmpCmd, uint16_t TmpMotorData)
{
    DebugP_log("print_GCode: %c %c %c %c\r\n", TmpCount, TmpCmd, (TmpMotorData>>8), (TmpMotorData & 0xff));
    if(TmpCount==0)
        return 1;
    if(TmpCmd==0)
        return 1;
    if((TmpMotorData >> 8)==0)
        return 1;
    if((TmpMotorData & 0xff)==0)
        return 1;
    return 0;
}

int motor_control_main(void)
{
    uint32_t    mcu_gpio0_BaseAddr;
    uint32_t    pin_step, pin_dir, count = 0;
    MotorMod MotorX, MotorY, MotorZ;;
    int32_t    lowest_width, width, peak_width;
    uint32_t    i, flag, rampcnt;

#ifdef MOTORX
    gpio_motor_control_init(&MotorX, true);
#else
    gpio_motor_control_init(&MotorX, false);
#endif
#ifdef MOTORY
    gpio_motor_control_init(&MotorY, true);
#else
    gpio_motor_control_init(&MotorY, false);
#endif
#ifdef MOTORZ
    gpio_motor_control_init(&MotorZ, true);
#else
    gpio_motor_control_init(&MotorZ, false);
#endif
    DebugP_log("Motor Control Started\r\n");

    /* Get address after translation translate */
    mcu_gpio0_BaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(GPIO_MOTOR_STEP_BASE_ADDR);
    pin_step       = GPIO_MOTOR_STEP_PIN;
    pin_dir        = GPIO_MOTOR_DIR_PIN;

    GPIO_setDirMode(mcu_gpio0_BaseAddr, pin_step, GPIO_MOTOR_STEP_DIR);
    GPIO_setDirMode(mcu_gpio0_BaseAddr, pin_dir, GPIO_MOTOR_DIR_DIR);

    i = 0;
    count = 10000;
    lowest_width = 1100;    //550;
    width = lowest_width;
    flag = 0;
    peak_width = 100;   //50;
    if(count > 1000)
        rampcnt = 500;
    else
        rampcnt = count / 2;

    rampcnt = rampcnt * SMOOTH;

    // 300/300 -> 1.6K
    // 100/400 -> 2K
    // 100/300 -> 2.5K
    // 50/300 -> 2.8K

    bMotorApplication = TRUE;
    while(count>0)
    {
        GPIO_pinWriteHigh(mcu_gpio0_BaseAddr, pin_step);
        //GPIO_pinWriteHigh(mcu_gpio0_BaseAddr, pin_dir);
        //ClockP_usleep(500);     // 32 micro step setup (with loading)
        ClockP_usleep(50);     // 16 micro step setup (with loading)
        GPIO_pinWriteLow(mcu_gpio0_BaseAddr, pin_step);

        if(i<rampcnt)
        {
            if(width > peak_width)
            {
                if(i%SMOOTH)
                    width = width - 1;
            }
        }
        else if(count<rampcnt)
        {
            if(width < lowest_width)
            {
                if(i%SMOOTH)
                    width = width + 1;
            }
        }


        ClockP_usleep(width);

        count--;
        i++;
    }

    return 0;
}
