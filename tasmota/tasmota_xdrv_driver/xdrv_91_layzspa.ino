/*
  xdrv_91_layzspa.ino - Lay Z Spa controller

  Copyright (C) 2021 Barbudor

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef USE_LAYZSPA

#define XDRV_91           91

#define CIO_DISPLAY_DATA_SIZE 12

struct LayZSpa {
  // CLK edge counter
  uint8_t cio_clk_bit;
  // Register that holds rx/tx bits
  uint8_t cio_data_reg;
  bool cio_tx;
  // Number of bytes processed since CS went low
  uint8_t cio_byte;
  // Current CIO command
  uint8_t cio_cmd;
  uint8_t cio_display_data[]
} *LAYZSPA = nullptr;

void LayZSpaInit() {
  LAYZSPA = (struct LayZSpa*) calloc(1, sizeof(struct LayZSpa));

  LAYZSPA->cio_clk_bit = 0;
  LAYZSPA->cio_data_reg = 0;
  LAYZSPA->cio_tx = false;

  if (PinUsed(GPIO_LAYZSPA_CIO_DATA) && PinUsed(GPIO_LAYZSPA_CIO_CLK) && PinUsed(GPIO_LAYZSPA_CIO_CS)) {
    AddLog(LOG_LEVEL_INFO, PSTR("LAYZSPA: CIO Init"));

    attachInterrupt(Pin(GPIO_LAYZSPA_CIO_CS), CioCsIsr, FALLING);
    attachInterrupt(Pin(GPIO_LAYZSPA_CIO_CLK), CioClkIsr, RISING);
    pinMode(Pin(GPIO_LAYZSPA_CIO_DATA), INPUT);
  }
  
}

void IRAM_ATTR CioCsIsr(void) {
  LAYZSPA->cio_clk_bit = 0;
  LAYZSPA->cio_data_reg = 0;
  LAYZSPA->cio_tx = false;
}

void IRAM_ATTR CioClkIsr(void) {
  if (LAYZSPA->cio_tx) {
    bool bit = (LAYZSPA->cio_data_reg >> LAYZSPA->cio_clk_bit) & 1;
    digitalWrite(Pin(GPIO_LAYZSPA_CIO_DATA), bit);
  } else {
    bool bit = digitalRead(Pin(GPIO_LAYZSPA_CIO_DATA));
    LAYZSPA->cio_data_reg |= bit << LAYZSPA->cio_clk_bit;
  }

  if (LAYZSPA->cio_clk_bit == 8) {
    CioHandleByte();
    LAYZSPA->cio_clk_bit = 0;
  } else {
    LAYZSPA->cio_clk_bit++;
  }
}

void IRAM_ATTR CioHandleByte() {
  if (LAYZSPA->cio_byte == 0) {
    LAYZSPA->cio_cmd = LAYZSPA->cio_data_reg;
  }

  if (LAYZSPA->cio_cmd == 0x40) {

  }

  LAYZSPA->cio_byte++;
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xdrv91(uint32_t function) {
  bool result = false;

  if (FUNC_INIT == function) {
    LayZSpaInit();
  } else if (LAYZSPA) {
    switch (function) {
      case FUNC_EVERY_SECOND:
        AddLog(LOG_LEVEL_INFO, PSTR("LAYZSPA: HELLO"));
        break;
    }
  }
  return result;
}

#endif // USE_LAYZSPA
