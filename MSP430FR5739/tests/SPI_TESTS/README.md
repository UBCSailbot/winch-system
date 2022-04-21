# ADC121S021

- Datasheet: https://www.ti.com/lit/ds/symlink/adc121s021.pdf?ts=1643368012541&ref_url=https%253A%252F%252Fwww.google.com%252F

The ADC121S021 is a single channel, 12 bit analog-to-digital converter. It is compatible with the SPI serial interface.

Two possible functional modes:

1. Normal mode - Enters mode when CS is pulled low
2. Shutdown mode - Enter mode when CS is pulled high before the 10th falling edge of SCLK after CS is pulled low.

If CS is brought high after the 10th falling edge, but before the 16th falling edge, the device remains in normal mode, but the current conversion is aborted.

Note:

- DATA is send on the falling edge of SCLK and is intended to be read by receiver on the rising edge. T

- The ADC provides three leading zeros followed by the 12 bit data(MSB)

## Operation order

1. Pull CS low which initiates a conversion process
2. 16 SCLK cycles are required to read a complete sample from the ADC.
3. Pull CS high after receiving 16 bits

## Tests

- [x] test_pot - Gets potentiometer data through SPI
- [x] test_hallConfig - Configures and verifies that the ADC was configured
