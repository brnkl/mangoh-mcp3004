#include "analoginputs.h"
#include "interfaces.h"
#include "legato.h"
#include "util.h"
#include "le_spiLibrary.h"

#define MAX_PATH_SIZE 256

#define SPI_BUFF_SIZE 3
#define N_CHANNELS 4
#define SPI_MODE 0
#define SPI_BITS 8
#define SPI_SPEED 960000
#define SPI_MSB 0

#define MCP30004_MULT 0.0298
#define MCP30004_OFFSET 0.0584

#define ANALOG_N_READINGS 10
#define ANALOG_DELAY_MSECS 200  // 200ms between readings
#define N_MAX_FAILED_READINGS 10

static const char* SPI_DEV_PATH = "/dev/spidev0.0";
int adcFd;

le_result_t analog_readVoltage(int chan, double* voltage) {
  uint32_t a2dVal = 0;
  uint8_t rxData[SPI_BUFF_SIZE];
  uint8_t txData[SPI_BUFF_SIZE];
  txData[0] = 0x1;  // start bit
  txData[1] =
      (~0)
      << 7;  // the source cited uses a slightly different value for txData[1]
  txData[1] |= (chan << 4);
  txData[2] = 0x0;

  le_result_t res = le_spiLib_WriteReadFD(adcFd, txData, rxData, SPI_BUFF_SIZE);

  // Originally this code masked rxData[1] first and then did the bit shifting
  // and bitwise or
  // but the source cited does the shift and bitwise or before masking
  a2dVal = ((rxData[1] << 8) | rxData[2]) & 0x3FF;
  *voltage = a2dVal * MCP30004_MULT + MCP30004_OFFSET;
  return res;
}

le_result_t analog_takeReadings(double* val, int chan) {
  double readings[ANALOG_N_READINGS];
  int i = 0, nFailed = 0;
  le_result_t r = LE_OK;
  while (i < ANALOG_N_READINGS) {
    r = analog_readVoltage(chan, &readings[i]);
    // only increment if the reading worked
    // but keep an eye on how many failures
    // such that this function will abort
    // on too many failures
    if (r == LE_OK) {
      i++;
    } else {
      if (nFailed > N_MAX_FAILED_READINGS) {
        return r;
      }
    }
    usleep(ANALOG_DELAY_MSECS * 1000);
  }

  *val = util_avgDouble(readings, ANALOG_N_READINGS);
  return r;
}

// Read from the MCP3004 analog to digital converter. This has four channels
le_result_t analog_readInputs(double* aIn, int nAIn) {
  le_result_t res[N_CHANNELS];
  for (int i = 0; i < nAIn; i++) {
    res[i] = analog_takeReadings(&aIn[i], i);
  }
  return util_flattenRes(res, N_CHANNELS);
}

void analog_setup() {
  adcFd = open(SPI_DEV_PATH, O_RDWR);
  le_spiLib_Configure(adcFd, SPI_MODE, SPI_BITS, SPI_SPEED, SPI_MSB);
}
