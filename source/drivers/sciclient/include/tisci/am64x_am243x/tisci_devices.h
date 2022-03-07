/*
 *  Copyright (C) 2017-2021 Texas Instruments Incorporated
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
 *
 */
/**
 * \ingroup TISCI
 * \defgroup tisci_devices tisci_devices
 *
 * DMSC controls the power management, security and resource management
 * of the device.
 *
 *
 * @{
 */
/**
 *
 *  \brief  This file contains:
 *
 *          WARNING!!: Autogenerated file from SYSFW. DO NOT MODIFY!!
 * Data version: 201208_205323
 *
 */
#ifndef SOC_TISCI_DEVICES_H
#define SOC_TISCI_DEVICES_H

#define TISCI_DEV_ADC0 0
#define TISCI_DEV_CMP_EVENT_INTROUTER0 1
#define TISCI_DEV_DBGSUSPENDROUTER0 2
#define TISCI_DEV_MAIN_GPIOMUX_INTROUTER0 3
#define TISCI_DEV_MCU_MCU_GPIOMUX_INTROUTER0 5
#define TISCI_DEV_TIMESYNC_EVENT_INTROUTER0 6
#define TISCI_DEV_MCU_M4FSS0 7
#define TISCI_DEV_MCU_M4FSS0_CORE0 9
#define TISCI_DEV_CPSW0 13
#define TISCI_DEV_CPT2_AGGR0 14
#define TISCI_DEV_STM0 15
#define TISCI_DEV_DCC0 16
#define TISCI_DEV_DCC1 17
#define TISCI_DEV_DCC2 18
#define TISCI_DEV_DCC3 19
#define TISCI_DEV_DCC4 20
#define TISCI_DEV_DCC5 21
#define TISCI_DEV_DMSC0 22
#define TISCI_DEV_MCU_DCC0 23
#define TISCI_DEV_DEBUGSS_WRAP0 24
#define TISCI_DEV_DMASS0 25
#define TISCI_DEV_DMASS0_BCDMA_0 26
#define TISCI_DEV_DMASS0_CBASS_0 27
#define TISCI_DEV_DMASS0_INTAGGR_0 28
#define TISCI_DEV_DMASS0_IPCSS_0 29
#define TISCI_DEV_DMASS0_PKTDMA_0 30
#define TISCI_DEV_DMASS0_PSILCFG_0 31
#define TISCI_DEV_DMASS0_PSILSS_0 32
#define TISCI_DEV_DMASS0_RINGACC_0 33
#define TISCI_DEV_MCU_TIMER0 35
#define TISCI_DEV_TIMER0 36
#define TISCI_DEV_TIMER1 37
#define TISCI_DEV_TIMER2 38
#define TISCI_DEV_TIMER3 39
#define TISCI_DEV_TIMER4 40
#define TISCI_DEV_TIMER5 41
#define TISCI_DEV_TIMER6 42
#define TISCI_DEV_TIMER7 43
#define TISCI_DEV_TIMER8 44
#define TISCI_DEV_TIMER9 45
#define TISCI_DEV_TIMER10 46
#define TISCI_DEV_TIMER11 47
#define TISCI_DEV_MCU_TIMER1 48
#define TISCI_DEV_MCU_TIMER2 49
#define TISCI_DEV_MCU_TIMER3 50
#define TISCI_DEV_ECAP0 51
#define TISCI_DEV_ECAP1 52
#define TISCI_DEV_ECAP2 53
#define TISCI_DEV_ELM0 54
#define TISCI_DEV_EMIF_DATA_0_VD 55
#define TISCI_DEV_MMCSD0 57
#define TISCI_DEV_MMCSD1 58
#define TISCI_DEV_EQEP0 59
#define TISCI_DEV_EQEP1 60
#define TISCI_DEV_GTC0 61
#define TISCI_DEV_EQEP2 62
#define TISCI_DEV_ESM0 63
#define TISCI_DEV_MCU_ESM0 64
#define TISCI_DEV_FSIRX0 65
#define TISCI_DEV_FSIRX1 66
#define TISCI_DEV_FSIRX2 67
#define TISCI_DEV_FSIRX3 68
#define TISCI_DEV_FSIRX4 69
#define TISCI_DEV_FSIRX5 70
#define TISCI_DEV_FSITX0 71
#define TISCI_DEV_FSITX1 72
#define TISCI_DEV_FSS0 73
#define TISCI_DEV_FSS0_FSAS_0 74
#define TISCI_DEV_FSS0_OSPI_0 75
#define TISCI_DEV_GICSS0 76
#define TISCI_DEV_GPIO0 77
#define TISCI_DEV_GPIO1 78
#define TISCI_DEV_MCU_GPIO0 79
#define TISCI_DEV_GPMC0 80
#define TISCI_DEV_PRU_ICSSG0 81
#define TISCI_DEV_PRU_ICSSG1 82
#define TISCI_DEV_LED0 83
#define TISCI_DEV_CPTS0 84
#define TISCI_DEV_DDPA0 85
#define TISCI_DEV_EPWM0 86
#define TISCI_DEV_EPWM1 87
#define TISCI_DEV_EPWM2 88
#define TISCI_DEV_EPWM3 89
#define TISCI_DEV_EPWM4 90
#define TISCI_DEV_EPWM5 91
#define TISCI_DEV_EPWM6 92
#define TISCI_DEV_EPWM7 93
#define TISCI_DEV_EPWM8 94
#define TISCI_DEV_VTM0 95
#define TISCI_DEV_MAILBOX0 96
#define TISCI_DEV_MAIN2MCU_VD 97
#define TISCI_DEV_MCAN0 98
#define TISCI_DEV_MCAN1 99
#define TISCI_DEV_MCU_MCRC64_0 100
#define TISCI_DEV_MCU2MAIN_VD 101
#define TISCI_DEV_I2C0 102
#define TISCI_DEV_I2C1 103
#define TISCI_DEV_I2C2 104
#define TISCI_DEV_I2C3 105
#define TISCI_DEV_MCU_I2C0 106
#define TISCI_DEV_MCU_I2C1 107
#define TISCI_DEV_MSRAM_256K0 108
#define TISCI_DEV_MSRAM_256K1 109
#define TISCI_DEV_MSRAM_256K2 110
#define TISCI_DEV_MSRAM_256K3 111
#define TISCI_DEV_MSRAM_256K4 112
#define TISCI_DEV_MSRAM_256K5 113
#define TISCI_DEV_PCIE0 114
#define TISCI_DEV_POSTDIV1_16FFT1 115
#define TISCI_DEV_POSTDIV4_16FF0 116
#define TISCI_DEV_POSTDIV4_16FF2 117
#define TISCI_DEV_PSRAMECC0 118
#define TISCI_DEV_R5FSS0 119
#define TISCI_DEV_R5FSS1 120
#define TISCI_DEV_R5FSS0_CORE0 121
#define TISCI_DEV_R5FSS0_CORE1 122
#define TISCI_DEV_R5FSS1_CORE0 123
#define TISCI_DEV_R5FSS1_CORE1 124
#define TISCI_DEV_RTI0 125
#define TISCI_DEV_RTI1 126
#define TISCI_DEV_RTI8 127
#define TISCI_DEV_RTI9 128
#define TISCI_DEV_RTI10 130
#define TISCI_DEV_RTI11 131
#define TISCI_DEV_MCU_RTI0 132
#define TISCI_DEV_SA2_UL0 133
#define TISCI_DEV_COMPUTE_CLUSTER0 134
#define TISCI_DEV_A53SS0_CORE_0 135
#define TISCI_DEV_A53SS0_CORE_1 136
#define TISCI_DEV_A53SS0 137
#define TISCI_DEV_DDR16SS0 138
#define TISCI_DEV_PSC0 139
#define TISCI_DEV_MCU_PSC0 140
#define TISCI_DEV_MCSPI0 141
#define TISCI_DEV_MCSPI1 142
#define TISCI_DEV_MCSPI2 143
#define TISCI_DEV_MCSPI3 144
#define TISCI_DEV_MCSPI4 145
#define TISCI_DEV_UART0 146
#define TISCI_DEV_MCU_MCSPI0 147
#define TISCI_DEV_MCU_MCSPI1 148
#define TISCI_DEV_MCU_UART0 149
#define TISCI_DEV_SPINLOCK0 150
#define TISCI_DEV_TIMERMGR0 151
#define TISCI_DEV_UART1 152
#define TISCI_DEV_UART2 153
#define TISCI_DEV_UART3 154
#define TISCI_DEV_UART4 155
#define TISCI_DEV_UART5 156
#define TISCI_DEV_BOARD0 157
#define TISCI_DEV_UART6 158
#define TISCI_DEV_MCU_UART1 160
#define TISCI_DEV_USB0 161
#define TISCI_DEV_SERDES_10G0 162
#define TISCI_DEV_PBIST0 163
#define TISCI_DEV_PBIST1 164
#define TISCI_DEV_PBIST2 165
#define TISCI_DEV_PBIST3 166
#define TISCI_DEV_COMPUTE_CLUSTER0_PBIST_0 167

#endif /* SOC_TISCI_DEVICES_H */

/** @} */
