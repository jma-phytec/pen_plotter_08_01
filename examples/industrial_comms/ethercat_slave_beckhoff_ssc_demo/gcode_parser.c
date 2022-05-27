/*
 *  Copyright (C) 2021 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/DebugP.h>
#include <drivers/ipc_notify.h>
#include <drivers/ipc_rpmsg.h>
#include <math.h>
#include <FreeRTOS.h>
#include <task.h>
#include <drivers/gpio.h>
#include <kernel/dpl/AddrTranslateP.h>
#include <tiescutils.h>
#include <tiesceoefoe.h>
#include <tiescsoc.h>
#include <applInterface.h>
#include <ecatslv.h>
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include "motor_control.h"
#include "gcode_data.h"

/* This example shows message exchange between multiple cores.
 *
 * One of the core is designated as the 'main' core
 * and other cores are designated as `remote` cores.
 *
 * The main core initiates IPC with remote core's by sending it a message.
 * The remote cores echo the same message to the main core.
 *
 * The main core repeats this for gMsgEchoCount iterations.
 *
 * In each iteration of message exchange, the message value is incremented.
 *
 * When iteration count reaches gMsgEchoCount, the example is completed.
 *
 */

/* number of iterations of message exchange to do */
uint32_t gMsgEchoCount = 462u;

#if defined(SOC_AM243X)
/* main core that starts the message exchange */
uint32_t gMainCoreId = CSL_CORE_ID_R5FSS0_0;
/* remote cores that echo messages from main core, make sure to NOT list main core in this list */
uint32_t gRemoteCoreId[] = {
    CSL_CORE_ID_M4FSS0_0,
    CSL_CORE_ID_R5FSS0_1,
    CSL_CORE_ID_R5FSS1_0,
    CSL_CORE_ID_R5FSS1_1,
    CSL_CORE_ID_MAX /* this value indicates the end of the array */
};
#endif

#if defined (SOC_AM263X)
/* main core that starts the message exchange */
uint32_t gMainCoreId = CSL_CORE_ID_R5FSS0_0;
/* remote cores that echo messages from main core, make sure to NOT list main core in this list */
uint32_t gRemoteCoreId[] = {
    CSL_CORE_ID_R5FSS0_1,
    CSL_CORE_ID_R5FSS1_0,
    CSL_CORE_ID_R5FSS1_1,
    CSL_CORE_ID_MAX /* this value indicates the end of the array */
};
#endif

#if defined(SOC_AM64X)
/* main core that starts the message exchange */
uint32_t gMainCoreId = CSL_CORE_ID_R5FSS0_0;
/* remote cores that echo messages from main core, make sure to NOT list main core in this list */
uint32_t gRemoteCoreId[] = {
//    CSL_CORE_ID_M4FSS0_0,
//    CSL_CORE_ID_R5FSS0_1,
    CSL_CORE_ID_R5FSS1_0,
//    CSL_CORE_ID_R5FSS1_1,
//    CSL_CORE_ID_A53SS0_0,
    CSL_CORE_ID_MAX /* this value indicates the end of the array */
};
#endif

#if defined(SOC_AM273X) || defined(SOC_AWR294X)
/* main core that starts the message exchange */
uint32_t gMainCoreId = CSL_CORE_ID_R5FSS0_0;
/* remote cores that echo messages from main core, make sure to NOT list main core in this list */
uint32_t gRemoteCoreId[] = {
    CSL_CORE_ID_R5FSS0_1,
    CSL_CORE_ID_C66SS0,
    CSL_CORE_ID_MAX /* this value indicates the end of the array */
};
#endif

/*
 * Remote core service end point
 *
 * pick any unique value on that core between 0..RPMESSAGE_MAX_LOCAL_ENDPT-1
 * the value need not be unique across cores
 */
uint16_t gRemoteServiceEndPt = 13u;

/* maximum size that message can have in this example */
#define MAX_MSG_SIZE        (128u)

/* Main core ack reply end point
 *
 * pick any unique value on that core between 0..RPMESSAGE_MAX_LOCAL_ENDPT-1
 * the value need not be unique across cores
 */
#define MAIN_CORE_ACK_REPLY_END_PT  (12U)

/* RPMessage_Object MUST be global or static */
RPMessage_Object gAckReplyMsgObject;

double to_degrees(double rad)
{
  return rad * 180.0 / M_PI;
}
double to_radians(double deg)
{
  return deg * M_PI / 180.0;
}
struct point
{
    double x;
    double y;
};
typedef struct point Point;

void DrawCur(Point SP, Point EP, double I, double J, uint8_t CW)
{
        Point CP, CurP;
        double R, X, Y, NewX, NewY;
        double a, a_r;
        int i;
        CurP.x = 2;
        CurP.y = 7;
        CP.x = SP.x + I;
        CP.y = SP.y + J;

        R = sqrt((I)*(I) + (J)*(J));
        DebugP_log("CP %f %f R %f\r\n", CP.x, CP.y, R);

        for(i=0; i<360; i++)
        {
            DebugP_log("\r\nCurP %f %f\r\n", CurP.x, CurP.y);

            if((CurP.x<CP.x) && (CurP.y>CP.y))
            {
                //DebugP_log("Quadrant 1\r\n");

                X = CP.x - CurP.x;
                Y = CurP.y - CP.y;
                X = fabs(X);
                Y = fabs(Y);
                a_r = asin(Y/R);
                a = to_degrees(a_r);
                //DebugP_log("X %f Y %f a_r %f a %f\r\n", X, Y, a_r, a);
                if(CW==0)
                    a--;
                else
                    a++;
                a_r = to_radians(a);
                NewX = R * cos(a_r);
                NewY = R * sin(a_r);
                //DebugP_log("NewP %f %f\r\n", NewX, NewY);
                CurP.x = CP.x - NewX;
                CurP.y = CP.y + NewY;
                //DebugP_log("Updated CurP %f %f\r\n", CurP.x, CurP.y);
            }
            else if((CurP.x>CP.x) && (CurP.y>CP.y))
            {
                //DebugP_log("Quadrant 2\r\n");
                X = CurP.x - CP.x;
                Y = CurP.y - CP.y;
                X = fabs(X);
                Y = fabs(Y);
                a_r = asin(Y/R);
                a = to_degrees(a_r);
                //DebugP_log("X %f Y %f a_r %f a %f\r\n", X, Y, a_r, a);
                if(CW==0)
                    a++;
                else
                    a--;
                a_r = to_radians(a);
                NewX = R * cos(a_r);
                NewY = R * sin(a_r);
                //DebugP_log("NewP %f %f\r\n", NewX, NewY);
                CurP.x = CP.x + NewX;
                CurP.y = CP.y + NewY;
                //DebugP_log("Updated CurP %f %f\r\n", CurP.x, CurP.y);
            }
            else if((CurP.x>CP.x) && (CurP.y<CP.y))
            {
                //DebugP_log("Quadrant 3\r\n");
                X = CurP.x - CP.x;
                Y = CP.y - CurP.y;
                X = fabs(X);
                Y = fabs(Y);
                a_r = asin(Y/R);
                a = to_degrees(a_r);
                //DebugP_log("X %f Y %f a_r %f a %f\r\n", X, Y, a_r, a);
                if(CW==0)
                    a--;
                else
                    a++;
                a_r = to_radians(a);
                NewX = R * cos(a_r);
                NewY = R * sin(a_r);
                //DebugP_log("NewP %f %f\r\n", NewX, NewY);
                CurP.x = CP.x + NewX;
                CurP.y = CP.y - NewY;
                //DebugP_log("Updated CurP %f %f\r\n", CurP.x, CurP.y);
            }
            else if((CurP.x<CP.x) && (CurP.y<CP.y))
            {
                //DebugP_log("Quadrant 4\r\n");
                X = CP.x - CurP.x;
                Y = CP.y - CurP.y;
                X = fabs(X);
                Y = fabs(Y);
                a_r = asin(Y/R);
                a = to_degrees(a_r);
                //DebugP_log("X %f Y %f a_r %f a %f\r\n", X, Y, a_r, a);
                if(CW==0)
                    a++;
                else
                    a--;
                a_r = to_radians(a);
                NewX = R * cos(a_r);
                NewY = R * sin(a_r);
                //DebugP_log("NewP %f %f\r\n", NewX, NewY);
                CurP.x = CP.x - NewX;
                CurP.y = CP.y - NewY;
                //DebugP_log("Updated CurP %f %f\r\n", CurP.x, CurP.y);
            }
            else
            {
                DebugP_log("Quadrant Unknown\r\n");
            }

            if((fabs(CurP.x-EP.x)<0.00001f) && (fabs(CurP.y-EP.y)<0.00001f))
            {
                DebugP_log("Hit EP\r\n");
                break;
            }
        }
}


void ipc_rpmsg_echo_main_core_start()
{
    RPMessage_CreateParams createParams;
    uint32_t msg, i, numRemoteCores;
    uint64_t curTime;
    char msgBuf[MAX_MSG_SIZE];
    int32_t status;
    uint16_t remoteCoreId, remoteCoreEndPt, msgSize;

    RPMessage_CreateParams_init(&createParams);
    createParams.localEndPt = MAIN_CORE_ACK_REPLY_END_PT;
    status = RPMessage_construct(&gAckReplyMsgObject, &createParams);
    DebugP_assert(status==SystemP_SUCCESS);

    numRemoteCores = 0;
    for(i=0; gRemoteCoreId[i]!=CSL_CORE_ID_MAX; i++)
    {
        numRemoteCores++;
    }

    /* wait for all cores to be ready */
    IpcNotify_syncAll(SystemP_WAIT_FOREVER);

    ClockP_usleep(500*1000); /* wait for log messages from remote cores to be flushed, otherwise this delay is not needed */

    DebugP_log("[IPC RPMSG ECHO] Message exchange started by main core !!!\r\n");

    curTime = ClockP_getTimeUsec();

    for(msg=0; msg<gMsgEchoCount; msg++)
    {
        //snprintf(msgBuf, MAX_MSG_SIZE-1, "%d", msg);
        memset(msgBuf, 0, MAX_MSG_SIZE-1);
        memcpy(msgBuf, GCodeFile1[msg], MAX_MSG_SIZE-1);
        msgBuf[MAX_MSG_SIZE-1] = 0;
        msgSize = strlen(msgBuf) + 1; /* count the terminating char as well */

        /* send the same messages to all cores */
        for(i=0; gRemoteCoreId[i]!=CSL_CORE_ID_MAX; i++ )
        {
            status = RPMessage_send(
                msgBuf, msgSize,
                gRemoteCoreId[i], gRemoteServiceEndPt,
                RPMessage_getLocalEndPt(&gAckReplyMsgObject),
                SystemP_WAIT_FOREVER);
            DebugP_assert(status==SystemP_SUCCESS);
        }
        /* wait for response from all cores */
        for(i=0; gRemoteCoreId[i]!=CSL_CORE_ID_MAX; i++ )
        {
            /* set 'msgSize' to size of recv buffer,
            * after return `msgSize` contains actual size of valid data in recv buffer
            */
            msgSize = sizeof(msgBuf);
            status = RPMessage_recv(&gAckReplyMsgObject,
                msgBuf, &msgSize,
                &remoteCoreId, &remoteCoreEndPt,
                SystemP_WAIT_FOREVER);
            DebugP_assert(status==SystemP_SUCCESS);
        }
    }

    curTime = ClockP_getTimeUsec() - curTime;

    DebugP_log("[IPC RPMSG ECHO] All echoed messages received by main core from %d remote cores !!!\r\n", numRemoteCores);
    DebugP_log("[IPC RPMSG ECHO] Messages sent to each core = %d \r\n", gMsgEchoCount);
    DebugP_log("[IPC RPMSG ECHO] Number of remote cores = %d \r\n", numRemoteCores);
    DebugP_log("[IPC RPMSG ECHO] Total execution time = %" PRId64 " usecs\r\n", curTime);
    DebugP_log("[IPC RPMSG ECHO] One way message latency = %" PRId32 " nsec\r\n",
        (uint32_t)(curTime*1000u/(gMsgEchoCount*numRemoteCores*2)));

    RPMessage_destruct(&gAckReplyMsgObject);

    DebugP_log("All tests have passed!!\r\n");
}

#define MAX_INT_DIGITS 8 // Maximum number of digits in int32 (and float)

uint8_t read_float(char *line, uint8_t *char_counter, float *float_ptr, Bool *isNegative)
{
  char *ptr = line + *char_counter;
  unsigned char c;

  // Grab first character and increment pointer. No spaces assumed in line.
  c = *ptr++;

  // Capture initial positive/minus character
  *isNegative = FALSE;
  if (c == '-') {
    *isNegative = TRUE;
    c = *ptr++;
  } else if (c == '+') {
    c = *ptr++;
  }

  // Extract number into fast integer. Track decimal in terms of exponent value.
  uint32_t intval = 0;
  int8_t exp = 0;
  uint8_t ndigit = 0;
  Bool isdecimal = FALSE;
  while(1) {
    c -= '0';
    if (c <= 9) {
      ndigit++;
      if (ndigit <= MAX_INT_DIGITS) {
        if (isdecimal) { exp--; }
        intval = (((intval << 2) + intval) << 1) + c; // intval*10 + c
      } else {
        if (!(isdecimal)) { exp++; }  // Drop overflow digits
      }
    } else if (c == (('.'-'0') & 0xff)  &&  !(isdecimal)) {
      isdecimal = TRUE;
    } else {
      break;
    }
    c = *ptr++;
  }

  // Return if no digits have been read.
  if (!ndigit) { return(FALSE); };

  // Convert integer into floating point.
  float fval;
  fval = (float)intval;

  // Apply decimal. Should perform no more than two floating point multiplications for the
  // expected range of E0 to E-4.
  if (fval != 0) {
    while (exp <= -2) {
      fval *= 0.01;
      exp += 2;
    }
    if (exp < 0) {
      fval *= 0.1;
    } else if (exp > 0) {
      do {
        fval *= 10.0;
      } while (--exp > 0);
    }
  }

  // Assign floating point value with correct sign.
  if (*isNegative) {
    *float_ptr = -fval;
  } else {
    *float_ptr = fval;
  }

  *char_counter = ptr - line - 1; // Set char_counter to next statement

  return(TRUE);
}


uint8_t gc_execute_line(MotorMod *MotorX, MotorMod *MotorY, MotorMod *MotorZ, char *line)
{
    uint8_t char_counter = 0;
    char letter;
    float value;
    uint8_t int_value = 0;
    uint16_t mantissa = 0;
    Bool isNegative = FALSE;
    Bool isNegativeX = FALSE;
    Bool isNegativeY = FALSE;
    Bool isNegativeZ = FALSE;
    Bool RequiredMoveX = FALSE;
    Bool RequiredMoveY = FALSE;
    Bool RequiredMoveZ = FALSE;

    if(line[0]=='$')
    {

    }

    while(line[char_counter] != 0)      // Parse whole line command
    {
        letter = line[char_counter];

        if((letter <'A') || (letter > 'Z'))
        {
            if(letter == ' ') // skip space char
            {
                char_counter++;
                continue;
            }
            if(letter == '%') // start of program
            {
                char_counter++;
                continue;
            }
        }

        char_counter++;
        //DebugP_log("char_counter %d\r\n", char_counter);


        if(!read_float(line, &char_counter, &value, &isNegative))   // Read command code
        {
            return 0;
        }
        int_value = trunc(value);
        mantissa =  round(100*(value - int_value)); // Compute mantissa for Gxx.x commands.

#ifdef MYDEBUG
        DebugP_log("value %f int_value %d mantissa %d\r\n",  value, int_value, mantissa);
#endif

        switch(letter)
        {
        case 'G':
            switch(int_value)
            {
            case 00:
                /*
                 *  G00 command moves the machine at maximum travel speed from a current position to a specified point
                 * or the coordinates specified by the command.
                 * The machine will move all axis at the same time so they complete the travel simultaneously.
                 * This results in a straight line movement to the new position point.
                 */
                break;
            case 01:
                /*
                 *  G01 G-code command instructs the machine to move in a straight line at a set feed rate or speed.
                 *  We specify the end position with the X, Y and Z values, and the speed with the F value.
                 */
                break;
            case 02:
                /*
                 * G02 command tells the machine to move clockwise in a circular pattern.
                 */
                break;
            case 03:
                /*
                 * G03 command tells the machine to move counterclockwise in a circular pattern.
                 */
                break;
            case 20:    // inches
            case 21:    // millimeters
                gpio_motor_control_ioctl(MotorX, UPDATE_UNIT, int_value);
                break;
            case 17:
            case 18:
            case 19:
                /* Note Supported: Plan Selection */
                break;
            case 28:    // return/set home
                break;
            case 90:    // absoulte mode
            case 91:    // relative mode
                gpio_motor_control_ioctl(MotorX, UPDATE_POSITIONING, int_value);
                break;
            case 92:    // set the current position as 0
                gpio_motor_control_ioctl(MotorX, UPDATE_HOME, int_value);
                break;
            default:
                break;
            }
            break;

        case 'M':
            switch(int_value)
            {
              case 0:
                  //ClockP_usleep(1000*100);
                  //ClockP_usleep(1000*100);
              case 2:
              case 30:
                  // Stop Program
                  break;
              default:
                  break;
            }
            break;

        default:
            switch(letter){
              case 'F': // Feedrate
              case 'S': // Speed
                  gpio_motor_control_setSpeed(MotorX, value);
                  break;
              case 'X':     // X Direction
#ifdef MOTORX
                  gpio_motor_control_setNextPos(MotorX, value, isNegative);
                  isNegativeX = isNegative;
                  RequiredMoveX = TRUE;
#endif
                  break;
              case 'Y':     // Y Direction
#ifdef MOTORY
                  gpio_motor_control_setNextPos(MotorX, value, isNegative);
                  isNegativeY = isNegative;
                  RequiredMoveY = TRUE;
#endif
                  break;
              case 'Z':     // Z Direction
#ifdef MOTORZ
                  gpio_motor_control_setNextPos(MotorX, value, isNegative);
                  isNegativeZ = isNegative;
                  RequiredMoveZ = TRUE;
#endif
                  break;
              default:
                  break;
            }
            break;
        }
    }


    if(RequiredMoveX)
    {
        gpio_motor_move(MotorX, isNegativeX);   // return after motor movement complete
        RequiredMoveX = FALSE;
    }
    if(RequiredMoveY)
    {
        gpio_motor_move(MotorX, isNegativeY);   // return after motor movement complete
        RequiredMoveY = FALSE;
    }
    if(RequiredMoveZ)
    {
        gpio_motor_move(MotorX, isNegativeZ);   // return after motor movement complete
        RequiredMoveZ = FALSE;
    }
    //DebugP_log("Position %d %d\r\n", MotorX->cur_pos, MotorY->cur_pos);

    return 0;
}

extern char msgBuf[128];

int motor_demo_main(void)
{
    //int i = 0;
    //char msgBuf[MAX_MSG_SIZE];
    MotorMod MotorX, MotorY, MotorZ;

    DebugP_log("Motor Demo Started\r\n");
    memset(msgBuf, 0, MAX_MSG_SIZE-1);

    gpio_motor_control_init(&MotorX, CSL_CORE_ID_R5FSS1_0);
    //gpio_motor_control_init(&MotorY);
    //gpio_motor_control_init(&MotorZ);

    bMotorApplication = TRUE;
    bGCodeCommandRunning = FALSE;
    //bGCodeCommandRunning = TRUE;
    do
    {
        if(bGCodeCommandRunning==TRUE)
        {
            bGCodeCommandRunning = FALSE;
            //bGCodeCommandRunning = TRUE
            //memcpy(msgBuf, GCodeFile1[i++], MAX_MSG_SIZE-1);
            //DebugP_log("Before gc_execute_line\r\n");
            gc_execute_line(&MotorX, &MotorY, &MotorZ, msgBuf);       // G Code Parser
            //DebugP_log("After gc_execute_line\r\n");
            //if(i>548)
                //i = 0;

            //memset(msgBuf, 0, MAX_MSG_SIZE-1);
            //msgBuf[MAX_MSG_SIZE-1] = 0;
        }
        else
            vTaskDelay(5);
    }
    while(bMotorApplication == TRUE);
    /* This loop will never exit */
    return 0;
}

