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
int idx = 0;

float CalculateMotorLoop(MotorMod *Motor, float Displayment)
{
    float StepsRequired = 0;
    // 200 steps = 1 rotation = 4.04cm = 40.4mm
    StepsRequired = (Displayment / ROUND_LEN) * NUM_STEPS_PER_ROTATION * STEPSIZE;
#ifdef MOTORX
    StepsRequired = StepsRequired * 2.5;
#endif
#ifdef MOTORY
    StepsRequired = StepsRequired * 2.5;
#endif
#ifdef MOTORZ
    StepsRequired = StepsRequired * 50;
#endif

#ifdef MYDEBUG
    DebugP_log("CalculateMotorLoop: Displayment %f StepsRequired %f\r\n", Displayment, StepsRequired);
#endif
    return StepsRequired;
}

void gpio_motor_move(MotorMod *Motor, Bool isNegative)
{
    float Displacement;
    float LoopRequired;

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
        Displacement = Motor->next_pos;
        if(!isNegative)
        {
            Motor->dir = 1;
            Motor->cur_pos = Motor->cur_pos + Motor->next_pos;
        }
        else
        {
            Motor->dir = 0;
            Motor->cur_pos = Motor->cur_pos - Motor->next_pos;
        }
        gpio_motor_control_dir_main(Motor);
    }

    LoopRequired = CalculateMotorLoop(Motor, fabs(Displacement));
    gpio_motor_control_step_main(Motor, LoopRequired);
#ifdef MYDEBUG
    DebugP_log("isNegative %d Motor->dir %d Motor->cur_pos %f LoopRequired %f \r\n", isNegative, Motor->dir, Motor->cur_pos, LoopRequired);
#endif
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
    return;
}

void gpio_motor_control_setCurPos(MotorMod *Motor, float val)
{
    Motor->cur_pos = val;
    return;
}

void gpio_motor_control_setNextPos(MotorMod *Motor, float val)
{
    Motor->next_pos = val;
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

void gpio_motor_control_init(MotorMod *Motor, uint32_t core_id)
{
    Motor->cur_pos = 0;
    Motor->next_pos = 0;
    Motor->positioning = 90;
    Motor->dir = 0;
    Motor->unit = 21;
    Motor->moving = 0;
    Motor->isActive = TRUE;
    if(core_id==CSL_CORE_ID_R5FSS1_0)
    {
        Motor->step_base_addr = GPIO_MOTOR_STEP_BASE_ADDR;
        Motor->step_pin = GPIO_MOTOR_STEP_PIN;
        Motor->step_dir = GPIO_MOTOR_STEP_DIR;
        Motor->dir_base_addr = GPIO_MOTOR_DIR_BASE_ADDR;
        Motor->dir_pin = GPIO_MOTOR_DIR_PIN;
        Motor->dir_dir = GPIO_MOTOR_DIR_DIR;
    }

    gpio_motor_control_setSpeed(Motor, 6000);   // 6000mm per min = 100mm per sec


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

    return;
}

void gpio_motor_control_step_main(MotorMod *Motor, float StepsRequired)
{
    uint32_t    loopcnt = (uint32_t)StepsRequired;
    uint32_t    gpioBaseAddr, pinNum;

    /* Get address after translation translate */
    gpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(Motor->step_base_addr);
    pinNum       = Motor->step_pin;

    //DebugP_log("GPIO1 STEP pin toggle starts (loopcnt %d) width %d...\r\n", loopcnt, (uint32_t)(Motor->pulse_width));

    GPIO_setDirMode(gpioBaseAddr, pinNum, Motor->step_dir);
    while(loopcnt>0)
    {
        if(Motor->isActive)
        {
            GPIO_pinWriteHigh(gpioBaseAddr, pinNum);
            //ClockP_usleep((uint32_t)(Motor->pulse_width));
            ClockP_usleep(300);
            GPIO_pinWriteLow(gpioBaseAddr, pinNum);
            //ClockP_usleep((uint32_t)(Motor->pulse_width));
            ClockP_usleep(300);
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

void update_gcode_cmdbuf(uint16_t TmpMotorData)
{
    msgBuf[idx] = (char)(TmpMotorData & 0xff);
    if(TmpMotorData == 0)
    {
#ifdef MYDEBUG
        DebugP_log("msgBug %s\r\n", msgBuf);
#endif
        idx = 0;
    }
    else
        idx++;

    return;
}

void print_TmpMotorData(uint16_t TmpMotorData)
{
    DebugP_log("TmpMotorData %d\r\n", TmpMotorData);
    return;
}

int motor_control_main(void)
{
    uint32_t    mcu_gpio0_BaseAddr;
    uint32_t    pin_step, pin_dir, count = 0;
    //MotorMod MotorY, MotorZ;
    MotorMod MotorX;

    gpio_motor_control_init(&MotorX, CSL_CORE_ID_R5FSS1_0);
    //gpio_motor_control_init(&MotorY);
    //gpio_motor_control_init(&MotorZ);
    DebugP_log("Motor Control Started\r\n");

    /* Get address after translation translate */
    mcu_gpio0_BaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(GPIO_MOTOR_STEP_BASE_ADDR);
    pin_step       = GPIO_MOTOR_STEP_PIN;
    pin_dir        = GPIO_MOTOR_DIR_PIN;

    GPIO_setDirMode(mcu_gpio0_BaseAddr, pin_step, GPIO_MOTOR_STEP_DIR);
    GPIO_setDirMode(mcu_gpio0_BaseAddr, pin_dir, GPIO_MOTOR_DIR_DIR);

    bMotorApplication = TRUE;
    do
    {
        GPIO_pinWriteHigh(mcu_gpio0_BaseAddr, pin_step);
        //GPIO_pinWriteHigh(mcu_gpio0_BaseAddr, pin_dir);
        //ClockP_usleep(500);     // 32 micro step setup (with loading)
        ClockP_usleep(300);     // 16 micro step setup (with loading)
        GPIO_pinWriteLow(mcu_gpio0_BaseAddr, pin_step);
        if(count>8100)  // 10cm
        {
            return 0;
            GPIO_pinWriteLow(mcu_gpio0_BaseAddr, pin_dir);
        }
        else
            GPIO_pinWriteHigh(mcu_gpio0_BaseAddr, pin_dir);
        ClockP_usleep(300);
        count++;
        if(count>14000)
            count = 0;
    }
    while(bMotorApplication == TRUE);

    return 0;
}
