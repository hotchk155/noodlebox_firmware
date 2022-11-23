/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v11.0
processor: MKE04Z128xxx4
package_id: MKE04Z128VQH4
mcu_data: ksdk2_0
processor_version: 11.0.1
pin_labels:
- {pin_num: '1', pin_signal: PTD1/KBI0_P25/FTM2_CH3/SPI1_MOSI, label: ENCODER1, identifier: ENCODER1}
- {pin_num: '2', pin_signal: PTD0/KBI0_P24/FTM2_CH2/SPI1_SCK, label: ENCODER2, identifier: ENCODER2}
- {pin_num: '18', pin_signal: PTB5/KBI0_P13/FTM2_CH5/SPI0_PCS/ACMP1_OUT, label: LED1, identifier: LED1}
- {pin_num: '20', pin_signal: PTC3/KBI0_P19/FTM2_CH3/ADC0_SE11, label: LED2, identifier: LED2}
- {pin_num: '21', pin_signal: PTC2/KBI0_P18/FTM2_CH2/ADC0_SE10, label: LED3, identifier: LED3}
- {pin_num: '52', pin_signal: PTC6/KBI0_P22/UART1_RX, label: KEYSCAN1, identifier: KEYSCAN1}
- {pin_num: '24', pin_signal: PTD5/KBI0_P29/PWT_IN0, label: P_KDAT, identifier: P_KDAT}
- {pin_num: '25', pin_signal: PTC1/KBI0_P17/FTM2_CH1/ADC0_SE9, label: P_KDAT, identifier: P_KDAT;P_KCLK}
- {pin_num: '26', pin_signal: PTC0/KBI0_P16/FTM2_CH0/ADC0_SE8, label: P_ARCK, identifier: P_ARCK}
- {pin_num: '31', pin_signal: PTB3/KBI0_P11/SPI0_MOSI/FTM0_CH1/ADC0_SE7, label: P_ADAT, identifier: P_ADAT}
- {pin_num: '32', pin_signal: PTB2/KBI0_P10/SPI0_SCK/FTM0_CH0/ADC0_SE6, label: P_ASCK, identifier: P_ASCK}
- {pin_num: '38', pin_signal: PTA6/KBI0_P6/FTM2_FLT1/ACMP1_IN0/ADC0_SE2, label: GATEOUT4, identifier: GATEOUT4}
- {pin_num: '44', pin_signal: PTD4/KBI0_P28, label: GATEOUT3, identifier: GATEOUT3}
- {pin_num: '45', pin_signal: PTD3/KBI0_P27/SPI1_PCS, label: GATEOUT2, identifier: GATEOUT2}
- {pin_num: '46', pin_signal: PTD2/KBI0_P26/SPI1_MISO, label: GATEOUT1, identifier: GATEOUT1}
- {pin_num: '50', pin_signal: PTA0/KBI0_P0/FTM0_CH0/I2C0_4WSCLOUT/ACMP0_IN0/ADC0_SE0, label: EXTSYNC, identifier: EXTSYNC}
- {pin_num: '54', pin_signal: PTE2/KBI1_P2/SPI0_MISO/PWT_IN0, label: POWER_CTRL, identifier: POWER_CTRL}
- {pin_num: '61', pin_signal: PTC5/KBI0_P21/FTM1_CH1/RTCO, label: SYNCOUT, identifier: SYNCOUT}
- {pin_num: '19', pin_signal: PTB4/KBI0_P12/FTM2_CH4/SPI0_MISO/ACMP1_IN2/NMI_b, label: KEYSCAN2, identifier: KEYSCAN2}
- {pin_num: '5', pin_signal: PTE7/KBI1_P7/TCLK2/FTM1_CH1, label: OFF_SWITCH, identifier: OFF_SWITCH}
- {pin_num: '51', pin_signal: PTC7/KBI0_P23/UART1_TX, label: AUXIN, identifier: AUXIN}
- {pin_num: '37', pin_signal: PTA7/KBI0_P7/FTM2_FLT2/ACMP1_IN1/ADC0_SE3, label: AUXOUT, identifier: AUXOUT}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "pin_mux.h"

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void)
{
    BOARD_InitPins_Release();
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins_Release:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '47', peripheral: I2C0, signal: SCL, pin_signal: PTA3/KBI0_P3/UART0_TX/I2C0_SCL}
  - {pin_num: '48', peripheral: I2C0, signal: SDA, pin_signal: PTA2/KBI0_P2/UART0_RX/I2C0_SDA}
  - {pin_num: '49', peripheral: GPIOA, signal: 'GPIO, 1', pin_signal: PTA1/KBI0_P1/FTM0_CH1/I2C0_4WSDAOUT/ACMP0_IN1/ADC0_SE1}
  - {pin_num: '38', peripheral: GPIOA, signal: 'GPIO, 6', pin_signal: PTA6/KBI0_P6/FTM2_FLT1/ACMP1_IN0/ADC0_SE2}
  - {pin_num: '32', peripheral: GPIOA, signal: 'GPIO, 10', pin_signal: PTB2/KBI0_P10/SPI0_SCK/FTM0_CH0/ADC0_SE6}
  - {pin_num: '31', peripheral: GPIOA, signal: 'GPIO, 11', pin_signal: PTB3/KBI0_P11/SPI0_MOSI/FTM0_CH1/ADC0_SE7}
  - {pin_num: '18', peripheral: GPIOA, signal: 'GPIO, 13', pin_signal: PTB5/KBI0_P13/FTM2_CH5/SPI0_PCS/ACMP1_OUT}
  - {pin_num: '26', peripheral: GPIOA, signal: 'GPIO, 16', pin_signal: PTC0/KBI0_P16/FTM2_CH0/ADC0_SE8}
  - {pin_num: '25', peripheral: GPIOA, signal: 'GPIO, 17', pin_signal: PTC1/KBI0_P17/FTM2_CH1/ADC0_SE9, identifier: P_KCLK}
  - {pin_num: '21', peripheral: GPIOA, signal: 'GPIO, 18', pin_signal: PTC2/KBI0_P18/FTM2_CH2/ADC0_SE10}
  - {pin_num: '20', peripheral: GPIOA, signal: 'GPIO, 19', pin_signal: PTC3/KBI0_P19/FTM2_CH3/ADC0_SE11}
  - {pin_num: '61', peripheral: GPIOA, signal: 'GPIO, 21', pin_signal: PTC5/KBI0_P21/FTM1_CH1/RTCO}
  - {pin_num: '52', peripheral: GPIOA, signal: 'GPIO, 22', pin_signal: PTC6/KBI0_P22/UART1_RX, direction: INPUT, pullup_enable: enable}
  - {pin_num: '2', peripheral: GPIOA, signal: 'GPIO, 24', pin_signal: PTD0/KBI0_P24/FTM2_CH2/SPI1_SCK, pullup_enable: enable}
  - {pin_num: '1', peripheral: GPIOA, signal: 'GPIO, 25', pin_signal: PTD1/KBI0_P25/FTM2_CH3/SPI1_MOSI, pullup_enable: enable}
  - {pin_num: '46', peripheral: GPIOA, signal: 'GPIO, 26', pin_signal: PTD2/KBI0_P26/SPI1_MISO}
  - {pin_num: '45', peripheral: GPIOA, signal: 'GPIO, 27', pin_signal: PTD3/KBI0_P27/SPI1_PCS}
  - {pin_num: '44', peripheral: GPIOA, signal: 'GPIO, 28', pin_signal: PTD4/KBI0_P28}
  - {pin_num: '24', peripheral: GPIOA, signal: 'GPIO, 29', pin_signal: PTD5/KBI0_P29/PWT_IN0}
  - {pin_num: '50', peripheral: KBI0, signal: 'P, 0', pin_signal: PTA0/KBI0_P0/FTM0_CH0/I2C0_4WSCLOUT/ACMP0_IN0/ADC0_SE0}
  - {pin_num: '11', peripheral: OSC, signal: EXTAL, pin_signal: PTB7/KBI0_P15/I2C0_SCL/EXTAL}
  - {pin_num: '12', peripheral: OSC, signal: XTAL, pin_signal: PTB6/KBI0_P14/I2C0_SDA/XTAL}
  - {pin_num: '34', peripheral: UART0, signal: RX, pin_signal: PTB0/KBI0_P8/UART0_RX/PWT_IN1/ADC0_SE4}
  - {pin_num: '33', peripheral: UART0, signal: TX, pin_signal: PTB1/KBI0_P9/UART0_TX/ADC0_SE5}
  - {pin_num: '19', peripheral: GPIOA, signal: 'GPIO, 12', pin_signal: PTB4/KBI0_P12/FTM2_CH4/SPI0_MISO/ACMP1_IN2/NMI_b, direction: INPUT, pullup_enable: enable}
  - {pin_num: '59', peripheral: I2C1, signal: SCL, pin_signal: PTE1/KBI1_P1/SPI0_MOSI/I2C1_SCL}
  - {pin_num: '5', peripheral: GPIOB, signal: 'GPIO, 7', pin_signal: PTE7/KBI1_P7/TCLK2/FTM1_CH1, direction: INPUT, pullup_enable: enable}
  - {pin_num: '51', peripheral: GPIOA, signal: 'GPIO, 23', pin_signal: PTC7/KBI0_P23/UART1_TX, direction: INPUT, pullup_enable: enable}
  - {pin_num: '37', peripheral: GPIOA, signal: 'GPIO, 7', pin_signal: PTA7/KBI0_P7/FTM2_FLT2/ACMP1_IN1/ADC0_SE3, direction: OUTPUT}
  - {pin_num: '54', peripheral: GPIOB, signal: 'GPIO, 2', pin_signal: PTE2/KBI1_P2/SPI0_MISO/PWT_IN0}
  - {pin_num: '60', peripheral: I2C1, signal: SDA, pin_signal: PTE0/KBI1_P0/SPI0_SCK/TCLK1/I2C1_SDA}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins_Release
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPins_Release(void)
{

    gpio_pin_config_t AUXOUT_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 0U
    };
    /* Initialize GPIO functionality on pin PTA7 (pin 37) */
    GPIO_PinInit(BOARD_INITPINS_RELEASE_AUXOUT_GPIO_PORT, BOARD_INITPINS_RELEASE_AUXOUT_PIN, &AUXOUT_config);

    gpio_pin_config_t KEYSCAN2_config = {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic = 0U
    };
    /* Initialize GPIO functionality on pin PTA12 (pin 19) */
    GPIO_PinInit(BOARD_INITPINS_RELEASE_KEYSCAN2_GPIO_PORT, BOARD_INITPINS_RELEASE_KEYSCAN2_PIN, &KEYSCAN2_config);

    gpio_pin_config_t KEYSCAN1_config = {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic = 0U
    };
    /* Initialize GPIO functionality on pin PTA22 (pin 52) */
    GPIO_PinInit(BOARD_INITPINS_RELEASE_KEYSCAN1_GPIO_PORT, BOARD_INITPINS_RELEASE_KEYSCAN1_PIN, &KEYSCAN1_config);

    gpio_pin_config_t AUXIN_config = {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic = 0U
    };
    /* Initialize GPIO functionality on pin PTA23 (pin 51) */
    GPIO_PinInit(BOARD_INITPINS_RELEASE_AUXIN_GPIO_PORT, BOARD_INITPINS_RELEASE_AUXIN_PIN, &AUXIN_config);

    gpio_pin_config_t OFF_SWITCH_config = {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic = 0U
    };
    /* Initialize GPIO functionality on pin PTB7 (pin 5) */
    GPIO_PinInit(BOARD_INITPINS_RELEASE_OFF_SWITCH_GPIO_PORT, BOARD_INITPINS_RELEASE_OFF_SWITCH_PIN, &OFF_SWITCH_config);
    /* Pull Enable for Port B Bit 4: 0x01u */
    PORT_SetPinPullUpEnable(PORT, kPORT_PTB, BOARD_INITPINS_RELEASE_KEYSCAN2_PIN, true);
    /* Pull Enable for Port C Bit 6: 0x01u */
    PORT_SetPinPullUpEnable(PORT, kPORT_PTC, BOARD_INITPINS_RELEASE_KEYSCAN1_PIN, true);
    /* Pull Enable for Port C Bit 7: 0x01u */
    PORT_SetPinPullUpEnable(PORT, kPORT_PTC, BOARD_INITPINS_RELEASE_AUXIN_PIN, true);
    /* Pull Enable for Port D Bit 0: 0x01u */
    PORT_SetPinPullUpEnable(PORT, kPORT_PTD, BOARD_INITPINS_RELEASE_ENCODER2_PIN, true);
    /* Pull Enable for Port D Bit 1: 0x01u */
    PORT_SetPinPullUpEnable(PORT, kPORT_PTD, BOARD_INITPINS_RELEASE_ENCODER1_PIN, true);
    /* Pull Enable for Port E Bit 7: 0x01u */
    PORT_SetPinPullUpEnable(PORT, kPORT_PTE, BOARD_INITPINS_RELEASE_OFF_SWITCH_PIN, true);
    /* pin 47,48 is configured as I2C0_SCL, I2C0_SDA */
    PORT_SetPinSelect(kPORT_I2C0, kPORT_I2C0_SCLPTA3_SDAPTA2);
    /* pin 34,33 is configured as UART0_RX, UART0_TX */
    PORT_SetPinSelect(kPORT_UART0, kPORT_UART0_RXPTB0_TXPTB1);
    /* pin 59,60 is configured as I2C1_SCL, I2C1_SDA */
    PORT_SetPinSelect(kPORT_I2C1, kPORT_I2C1_SCLPTE1_SDAPTE0);
    /* disable NMI function on pin 19 */
    PORT_SetPinSelect(kPORT_NMI, kPORT_NMI_OTHERS);

    SIM->SOPT0 = ((SIM->SOPT0 &
                   /* Mask bits to zero which are setting */
                   (~(SIM_SOPT0_RXDFE_MASK)))

                  /* UART0 RxD Filter Select: RXD0 input signal is connected to UART0 module directly. */
                  | SIM_SOPT0_RXDFE(SOPT0_RXDFE_0b00));
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
