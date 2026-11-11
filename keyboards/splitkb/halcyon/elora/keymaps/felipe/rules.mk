ENCODER_MAP_ENABLE = yes

# Profile indicator on the Halcyon TFT display
QUANTUM_PAINTER_ENABLE = yes
QUANTUM_PAINTER_DRIVERS += st7789_spi
SRC += display.c graphics/big.qff.c graphics/small.qff.c

# Cirque trackpad (right half)
POINTING_DEVICE_ENABLE = yes
POINTING_DEVICE_DRIVER = cirque_pinnacle_spi
