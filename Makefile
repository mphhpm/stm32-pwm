
all:dtmf-build

libopencm3-build:
	$(MAKE) all -C libopencm3 -j 16

libopencm3-clean:
	$(MAKE) clean -C libopencm3 -j16
	
dtmf-build:
	$(MAKE) all -C dtmf -j 8
	
dtmf-clean:
	$(MAKE) clean -C dtmf -j 8

dtmf-flash: dtmf-build
	st-flash --reset write dtmf/stm32-dtmf.bin 0x8000000
