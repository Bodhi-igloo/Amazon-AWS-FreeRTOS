/*
 * Copyright (c) 2013 - 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * FreeRTOS V1.4.7
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/*///////////////////////////////////////////////////////////////////////////// */
/*  Includes */
/*///////////////////////////////////////////////////////////////////////////// */

/* SDK Included Files */
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_power.h"
#include "usb_device_config.h"

#include "pin_mux.h"
#include <stdbool.h>

/* FreeRTOS Demo Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "aws_clientcredential.h"
#include "iot_logging_task.h"
#include "iot_wifi.h"
#include "iot_system_init.h"
#include "aws_demo.h"
#include "aws_dev_mode_key_provisioning.h"

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "virtual_com.h"
#include "fsl_power.h"
#include "mflash_drv.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* The lpc54018 usb logging driver calls a blocking write function. Since this
 * task is the lowest priority, all of the demo's priorities must be higher than
 * this to run. */
#define mainLOGGING_TASK_PRIORITY        ( tskIDLE_PRIORITY )
#define mainLOGGING_TASK_STACK_SIZE      ( configMINIMAL_STACK_SIZE * 4 )
#define mainLOGGING_QUEUE_LENGTH         ( 16 )

/* The task delay for allowing the lower priority logging task to print out Wi-Fi
 * failure status before blocking indefinitely. */
#define mainLOGGING_WIFI_STATUS_DELAY    pdMS_TO_TICKS( 1000 )

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void prvWifiConnect( void );

/*******************************************************************************
 * Code
 ******************************************************************************/

extern usb_cdc_vcom_struct_t s_cdcVcom;

#if ( defined( USB_DEVICE_CONFIG_LPCIP3511FS ) && ( USB_DEVICE_CONFIG_LPCIP3511FS > 0U ) )
    /*-----------------------------------------------------------*/

    void USB0_IRQHandler( void )
    {
        USB_DeviceLpcIp3511IsrFunction( s_cdcVcom.deviceHandle );
    }
#endif
#if ( defined( USB_DEVICE_CONFIG_LPCIP3511HS ) && ( USB_DEVICE_CONFIG_LPCIP3511HS > 0U ) )
    /*-----------------------------------------------------------*/

    void USB1_IRQHandler( void )
    {
        USB_DeviceLpcIp3511IsrFunction( s_cdcVcom.deviceHandle );
    }
#endif
/*-----------------------------------------------------------*/

void USB_DeviceClockInit( void )
{
    #if defined( USB_DEVICE_CONFIG_LPCIP3511FS ) && ( USB_DEVICE_CONFIG_LPCIP3511FS > 0U )
        /* enable USB IP clock */
        CLOCK_EnableUsbfs0DeviceClock( kCLOCK_UsbSrcFro, CLOCK_GetFroHfFreq() );
        #if defined( FSL_FEATURE_USB_USB_RAM ) && ( FSL_FEATURE_USB_USB_RAM )
            for( int i = 0; i < FSL_FEATURE_USB_USB_RAM; i++ )
            {
                ( ( uint8_t * ) FSL_FEATURE_USB_USB_RAM_BASE_ADDRESS )[ i ] = 0x00U;
            }
        #endif
    #endif
    #if defined( USB_DEVICE_CONFIG_LPCIP3511HS ) && ( USB_DEVICE_CONFIG_LPCIP3511HS > 0U )
        /* enable USB IP clock */
        CLOCK_EnableUsbhs0DeviceClock( kCLOCK_UsbSrcUsbPll, 0U );
        #if defined( FSL_FEATURE_USBHSD_USB_RAM ) && ( FSL_FEATURE_USBHSD_USB_RAM )
            for( int i = 0; i < FSL_FEATURE_USBHSD_USB_RAM; i++ )
            {
                ( ( uint8_t * ) FSL_FEATURE_USBHSD_USB_RAM_BASE_ADDRESS )[ i ] = 0x00U;
            }
        #endif
    #endif
}
/*-----------------------------------------------------------*/

void USB_DeviceIsrEnable( void )
{
    uint8_t irqNumber;

    #if defined( USB_DEVICE_CONFIG_LPCIP3511FS ) && ( USB_DEVICE_CONFIG_LPCIP3511FS > 0U )
        uint8_t usbDeviceIP3511Irq[] = USB_IRQS;
        irqNumber = usbDeviceIP3511Irq[ CONTROLLER_ID - kUSB_ControllerLpcIp3511Fs0 ];
    #endif
    #if defined( USB_DEVICE_CONFIG_LPCIP3511HS ) && ( USB_DEVICE_CONFIG_LPCIP3511HS > 0U )
        uint8_t usbDeviceIP3511Irq[] = USBHSD_IRQS;
        irqNumber = usbDeviceIP3511Irq[ CONTROLLER_ID - kUSB_ControllerLpcIp3511Hs0 ];
    #endif
    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority( ( IRQn_Type ) irqNumber, USB_DEVICE_INTERRUPT_PRIORITY );
    EnableIRQ( ( IRQn_Type ) irqNumber );
}


/*-----------------------------------------------------------*/

int main( void )
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk( BOARD_DEBUG_UART_CLK_ATTACH );

    /* reset USB0 and USB1 device */
    RESET_PeripheralReset( kUSB0D_RST_SHIFT_RSTn );
    RESET_PeripheralReset( kUSB1D_RST_SHIFT_RSTn );
    RESET_PeripheralReset( kUSB0HMR_RST_SHIFT_RSTn );
    RESET_PeripheralReset( kUSB0HSL_RST_SHIFT_RSTn );
    RESET_PeripheralReset( kUSB1H_RST_SHIFT_RSTn );

    NVIC_ClearPendingIRQ( USB0_IRQn );
    NVIC_ClearPendingIRQ( USB0_NEEDCLK_IRQn );
    NVIC_ClearPendingIRQ( USB1_IRQn );
    NVIC_ClearPendingIRQ( USB1_NEEDCLK_IRQn );

    BOARD_InitPins();
    BOARD_BootClockFROHF96M();

    #if ( defined USB_DEVICE_CONFIG_LPCIP3511HS ) && ( USB_DEVICE_CONFIG_LPCIP3511HS )
        POWER_DisablePD( kPDRUNCFG_PD_USB1_PHY );
        /* enable usb1 host clock */
        CLOCK_EnableClock( kCLOCK_Usbh1 );
        /*According to reference mannual, device mode setting has to be set by access usb host register */
        *( ( uint32_t * ) ( USBHSH_BASE + 0x50 ) ) |= USBHSH_PORTMODE_DEV_ENABLE_MASK;
        /* enable usb1 host clock */
        CLOCK_DisableClock( kCLOCK_Usbh1 );
    #endif
    #if ( defined USB_DEVICE_CONFIG_LPCIP3511FS ) && ( USB_DEVICE_CONFIG_LPCIP3511FS )
        POWER_DisablePD( kPDRUNCFG_PD_USB0_PHY ); /*< Turn on USB Phy */
        CLOCK_SetClkDiv( kCLOCK_DivUsb0Clk, 1, false );
        CLOCK_AttachClk( kFRO_HF_to_USB0_CLK );
        /* enable usb0 host clock */
        CLOCK_EnableClock( kCLOCK_Usbhsl0 );
        /*According to reference mannual, device mode setting has to be set by access usb host register */
        *( ( uint32_t * ) ( USBFSH_BASE + 0x5C ) ) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
        /* disable usb0 host clock */
        CLOCK_DisableClock( kCLOCK_Usbhsl0 );
    #endif

    BOARD_InitDebugConsole();

    /* Initialize FLASH driver */
    mflash_drv_init();

    xLoggingTaskInitialize( mainLOGGING_TASK_STACK_SIZE,
                            mainLOGGING_TASK_PRIORITY,
                            mainLOGGING_QUEUE_LENGTH );

    vTaskStartScheduler();

    for( ; ; )
    {
    }
}

/*-----------------------------------------------------------*/

void vApplicationDaemonTaskStartupHook( void )
{
    WIFIReturnCode_t xWifiStatus;
    /* A simple example to demonstrate key and certificate provisioning in
     * microcontroller flash using PKCS#11 interface. This should be replaced
     * by production ready key provisioning mechanism. */
    vDevModeKeyProvisioning();

    xWifiStatus = WIFI_On();

    if( xWifiStatus == eWiFiSuccess )
    {
        configPRINTF( ( "WiFi module initialized.\r\n" ) );

        if( SYSTEM_Init() == pdPASS )
        {
        
            prvWifiConnect();

            DEMO_RUNNER_RunDemos();
        }
   }
   else
   {
       configPRINTF( ( "WiFi module failed to initialize.\r\n" ) );
       /* Stop here if we fail to initialize WiFi. */
       configASSERT( xWifiStatus == eWiFiSuccess );
   }
       
}

/*-----------------------------------------------------------*/

static void prvWifiConnect( void )
{
    WIFINetworkParams_t xNetworkParams = { 0 };
    WIFIReturnCode_t xWifiStatus = eWiFiSuccess;
    WIFIIPConfiguration_t xIpConfig;
    uint8_t *pucIPV4Byte;
    size_t xSSIDLength, xPasswordLength;

    /* Setup WiFi parameters to connect to access point. */
    if( clientcredentialWIFI_SSID != NULL )
    {
        xSSIDLength = strlen( clientcredentialWIFI_SSID );
        if( ( xSSIDLength > 0 ) && ( xSSIDLength <= wificonfigMAX_SSID_LEN ) )
        {
            xNetworkParams.ucSSIDLength = xSSIDLength;
            memcpy( xNetworkParams.ucSSID, clientcredentialWIFI_SSID, xSSIDLength );
        }
        else
        {
            configPRINTF(( "Invalid WiFi SSID configured, empty or exceeds allowable length %d.\n", wificonfigMAX_SSID_LEN ));
            xWifiStatus = eWiFiFailure;
        }
    }
    else
    {
        configPRINTF(( "WiFi SSID is not configured.\n" ));
        xWifiStatus = eWiFiFailure;
    }

    xNetworkParams.xSecurity = clientcredentialWIFI_SECURITY;
    if( clientcredentialWIFI_SECURITY == eWiFiSecurityWPA2 )
    {
        if( clientcredentialWIFI_PASSWORD != NULL )
        {
            xPasswordLength = strlen( clientcredentialWIFI_PASSWORD );
            if( ( xPasswordLength > 0 ) && ( xSSIDLength <= wificonfigMAX_PASSPHRASE_LEN ) )
            {
                xNetworkParams.xPassword.xWPA.ucLength = xPasswordLength;
                memcpy( xNetworkParams.xPassword.xWPA.cPassphrase, clientcredentialWIFI_PASSWORD, xPasswordLength );
            }
            else
            {
                configPRINTF(( "Invalid WiFi password configured, empty password or exceeds allowable password length %d.\n", wificonfigMAX_PASSPHRASE_LEN ));
                xWifiStatus = eWiFiFailure;
            }
        }
        else
        {
            configPRINTF(( "WiFi password is not configured.\n" ));
            xWifiStatus = eWiFiFailure;
        }
    }
    xNetworkParams.ucChannel = 0;

    if( xWifiStatus == eWiFiSuccess )
    {
        /* Try connecting using provided wifi credentials. */
        xWifiStatus = WIFI_ConnectAP( &( xNetworkParams ) );

        if( xWifiStatus == eWiFiSuccess )
        {
            configPRINTF( ( "WiFi connected to AP %.*s.\r\n", xNetworkParams.ucSSIDLength, ( char * ) xNetworkParams.ucSSID ) );

            /* Get IP address of the device. */
            xWifiStatus = WIFI_GetIPInfo( &xIpConfig );
            if( xWifiStatus == eWiFiSuccess )
            {
                pucIPV4Byte = ( uint8_t * ) ( &xIpConfig.xIPAddress.ulAddress[0] );
                configPRINTF( ( "IP Address acquired %d.%d.%d.%d.\r\n",
                        pucIPV4Byte[ 0 ], pucIPV4Byte[ 1 ], pucIPV4Byte[ 2 ], pucIPV4Byte[ 3 ] ) );
            }
        }
        else
        {
            /* Connection failed configure softAP to allow user to set wifi credentials. */
            configPRINTF( ( "WiFi failed to connect to AP %.*s.\r\n", xNetworkParams.ucSSIDLength, ( char * ) xNetworkParams.ucSSID  ) );
        }
    }

    if( xWifiStatus != eWiFiSuccess )
    {
        /* Enter SOFT AP mode to provision access point credentials. */
        configPRINTF(( "Entering soft access point WiFi provisioning mode.\n" ));
        xWifiStatus = eWiFiSuccess;
        if( wificonfigACCESS_POINT_SSID_PREFIX != NULL )
        {
            xSSIDLength = strlen( wificonfigACCESS_POINT_SSID_PREFIX );
            if( ( xSSIDLength > 0 ) && ( xSSIDLength <= wificonfigMAX_SSID_LEN ) )
            {
                xNetworkParams.ucSSIDLength = xSSIDLength;
                memcpy( xNetworkParams.ucSSID, clientcredentialWIFI_SSID, xSSIDLength );
            }
            else
            {
                configPRINTF(( "Invalid AP SSID configured, empty or exceeds allowable length %d.\n", wificonfigMAX_SSID_LEN ));
                xWifiStatus = eWiFiFailure;
            }
        }
        else
        {
            configPRINTF(( "WiFi AP SSID is not configured.\n" ));
            xWifiStatus = eWiFiFailure;
        }

        xNetworkParams.xSecurity = wificonfigACCESS_POINT_SECURITY;
        if( wificonfigACCESS_POINT_SECURITY == eWiFiSecurityWPA2 )
        {
            if( wificonfigACCESS_POINT_PASSKEY != NULL )
            {
                xPasswordLength = strlen( wificonfigACCESS_POINT_PASSKEY );
                if( ( xPasswordLength > 0 ) && ( xSSIDLength <= wificonfigMAX_PASSPHRASE_LEN ) )
                {
                    xNetworkParams.xPassword.xWPA.ucLength = xPasswordLength;
                    memcpy( xNetworkParams.xPassword.xWPA.cPassphrase, wificonfigACCESS_POINT_PASSKEY, xPasswordLength );
                }
                else
                {
                    configPRINTF(( "Invalid WiFi AP password, empty or exceeds allowable password length %d.\n", wificonfigMAX_PASSPHRASE_LEN ));
                    xWifiStatus = eWiFiFailure;
                }
            }
            else
            {
                configPRINTF(( "WiFi AP password is not configured.\n" ));
                xWifiStatus = eWiFiFailure;
            }
        }
        xNetworkParams.ucChannel = wificonfigACCESS_POINT_CHANNEL;

        if( xWifiStatus == eWiFiSuccess )
        {
            configPRINTF( ( "Connect to softAP %.*s using password %.*s. \r\n",
                    xNetworkParams.ucSSIDLength, ( char * ) xNetworkParams.ucSSID,
                    xNetworkParams.xPassword.xWPA.ucLength, xNetworkParams.xPassword.xWPA.cPassphrase ) );
            do
            {
                xWifiStatus = WIFI_ConfigureAP( &xNetworkParams );
                configPRINTF( ( "Connect to softAP and configure wiFi returned %d\r\n", xWifiStatus ) );
                vTaskDelay( pdMS_TO_TICKS( 1000 ) );

            } while( ( xWifiStatus != eWiFiSuccess ) && ( xWifiStatus != eWiFiNotSupported ) );
        }
    }

    configASSERT( xWifiStatus == eWiFiSuccess );

}

/*-----------------------------------------------------------*/

void * pvPortCalloc( size_t xNum,
                     size_t xSize )
{
    void * pvReturn;

    pvReturn = pvPortMalloc( xNum * xSize );

    if( pvReturn != NULL )
    {
        memset( pvReturn, 0x00, xNum * xSize );
    }

    return pvReturn;
}