# Gear Motor (Pawl control)

## Configuration

- Three diffirent speed modes - FAST (90% duty), MEDIUM (50% duty) and SLOW (20% duty)
- Option for a motor timeout period

## Running tests

Tests can be selected by changing the test_sel variable to any of the defined tests below:

- MOVE_TIME_T - Moves the gear motor using timer delays
- MOVE_SPI_DATA_T - Moves the gear motor using spi data
