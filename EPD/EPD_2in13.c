#include "EPD_2in13.h"

#define EPD_2IN13_IMAGE_BYTES  ((EPD_2IN13_WIDTH / 8) * EPD_2IN13_HEIGHT)

static const unsigned char EPD_2IN13_lut_20_vcom0_full[] = {
    0x00, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x60, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,
};

static const unsigned char EPD_2IN13_lut_21_ww_full[] = {
    0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
    0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char EPD_2IN13_lut_22_bw_full[] = {
    0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
    0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char EPD_2IN13_lut_23_wb_full[] = {
    0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char EPD_2IN13_lut_24_bb_full[] = {
    0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static void EPD_2IN13_Reset(void)
{
    DEV_Digital_Write(EPD_RST_PIN, 0);
    DEV_Delay_ms(10);
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(10);
}

void EPD_2IN13_SendCommand(UBYTE Reg)
{
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Reg);
    DEV_Digital_Write(EPD_CS_PIN, 1);
    DEV_Digital_Write(EPD_DC_PIN, 1);
}

void EPD_2IN13_SendData(UBYTE Data)
{
    DEV_Digital_Write(EPD_DC_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

static void EPD_2IN13_ReadBusy(void)
{
    while (DEV_Digital_Read(EPD_BUSY_PIN) == 0)
    {
        DEV_Delay_ms(10);
    }
}

static void EPD_2IN13_TurnOnDisplay(void)
{
    EPD_2IN13_SendCommand(0x12);
    EPD_2IN13_ReadBusy();
}

static void EPD_2IN13_SendLut(const unsigned char *lut, UWORD length)
{
    for (UWORD i = 0; i < length; i++)
    {
        EPD_2IN13_SendData(lut[i]);
    }
}

void EPD_2IN13_Init(void)
{
    nrf_gpio_cfg_input(EPD_BUSY_PIN, NRF_GPIO_PIN_PULLUP);

    EPD_2IN13_Reset();
    EPD_2IN13_ReadBusy();

    EPD_2IN13_SendCommand(0x01); // POWER SETTING
    EPD_2IN13_SendData(0x03);
    EPD_2IN13_SendData(0x00);
    EPD_2IN13_SendData(0x2B);
    EPD_2IN13_SendData(0x2B);
    EPD_2IN13_SendData(0x03);

    EPD_2IN13_SendCommand(0x06); // BOOSTER_SOFT_START
    EPD_2IN13_SendData(0x17);
    EPD_2IN13_SendData(0x17);
    EPD_2IN13_SendData(0x17);

    EPD_2IN13_SendCommand(0x00); // PANEL_SETTING
    EPD_2IN13_SendData(0xB7);
    EPD_2IN13_SendData(0x0D);

    EPD_2IN13_SendCommand(0x30); // PLL_CONTROL
    EPD_2IN13_SendData(0x3A);

    EPD_2IN13_SendCommand(0x61); // RESOLUTION_SETTING
    EPD_2IN13_SendData(EPD_2IN13_WIDTH);
    EPD_2IN13_SendData(EPD_2IN13_HEIGHT / 256);
    EPD_2IN13_SendData(EPD_2IN13_HEIGHT % 256);

    EPD_2IN13_SendCommand(0x82); // VCM_DC_SETTING
    EPD_2IN13_SendData(0x1C);

    EPD_2IN13_SendCommand(0x50); // VCOM_AND_DATA_INTERVAL_SETTING
    EPD_2IN13_SendData(0x17);

    EPD_2IN13_SendCommand(0x20);
    EPD_2IN13_SendLut(EPD_2IN13_lut_20_vcom0_full, sizeof(EPD_2IN13_lut_20_vcom0_full));
    EPD_2IN13_SendCommand(0x21);
    EPD_2IN13_SendLut(EPD_2IN13_lut_21_ww_full, sizeof(EPD_2IN13_lut_21_ww_full));
    EPD_2IN13_SendCommand(0x22);
    EPD_2IN13_SendLut(EPD_2IN13_lut_22_bw_full, sizeof(EPD_2IN13_lut_22_bw_full));
    EPD_2IN13_SendCommand(0x23);
    EPD_2IN13_SendLut(EPD_2IN13_lut_23_wb_full, sizeof(EPD_2IN13_lut_23_wb_full));
    EPD_2IN13_SendCommand(0x24);
    EPD_2IN13_SendLut(EPD_2IN13_lut_24_bb_full, sizeof(EPD_2IN13_lut_24_bb_full));

    EPD_2IN13_SendCommand(0x04); // POWER_ON
    EPD_2IN13_ReadBusy();
}

void EPD_2IN13_Clear(void)
{
    EPD_2IN13_SendCommand(0x10); // OLD_RAM
    for (UWORD i = 0; i < EPD_2IN13_IMAGE_BYTES; i++)
    {
        EPD_2IN13_SendData(0x00);
    }

    EPD_2IN13_SendCommand(0x13); // NEW_RAM
    for (UWORD i = 0; i < EPD_2IN13_IMAGE_BYTES; i++)
    {
        EPD_2IN13_SendData(0xFF);
    }

    EPD_2IN13_TurnOnDisplay();
}

void EPD_2IN13_Display(UBYTE *Image)
{
    EPD_2IN13_SendCommand(0x10); // OLD_RAM
    for (UWORD i = 0; i < EPD_2IN13_IMAGE_BYTES; i++)
    {
        EPD_2IN13_SendData(0xFF);
    }

    EPD_2IN13_SendCommand(0x13); // NEW_RAM
    for (UWORD i = 0; i < EPD_2IN13_IMAGE_BYTES; i++)
    {
        EPD_2IN13_SendData(Image[i]);
    }

    EPD_2IN13_UpdateDisplay();
}

void EPD_2IN13_UpdateDisplay(void)
{
    EPD_2IN13_SendCommand(0x11); // DATA_STOP
    EPD_2IN13_TurnOnDisplay();
}

void EPD_2IN13_Sleep(void)
{
    EPD_2IN13_SendCommand(0x50);
    EPD_2IN13_SendData(0xF7);
    EPD_2IN13_SendCommand(0x02);
    EPD_2IN13_ReadBusy();
    EPD_2IN13_SendCommand(0x07);
    EPD_2IN13_SendData(0xA5);
}
