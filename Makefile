##
##    Copyright (c) 2024 mphhpm.
##    Permission is hereby granted, free of charge, to any person obtaining
##    a copy of this software and associated documentation files (the "Software"),
##    to deal in the Software without restriction, including without limitation
##    the rights to use, copy, modify, merge, publish, distribute, sublicense,
##    and/or sell copies of the Software, and to permit persons to whom the Software
##    is furnished to do so, subject to the following conditions:
##
##    The above copyright notice and this permission notice shall be included in
##    all copies or substantial portions of the Software.
##
##    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
##    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
##    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
##    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
##    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
##    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
##    OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
##
##    Changes
##    1-1-2024: mphhpm: initial version


export CYGWIN_DIR=C:/cygwin64/bin
export PROJECT = stm32-pwm
export BUILD_DIR =$(shell $(CYGWIN_DIR)/cygpath -m $(PWD)/./build/$(PROJECT)/obj)
export OUTPUT_DIR=$(shell $(CYGWIN_DIR)/cygpath -m $(PWD)/./build/$(PROJECT)/bin)
##
## ensure no optimization when debugging
export OPT = -O0
CPWD=$(shell $(CYGWIN_DIR)/cygpath -m $(PWD))
##
## set verbosity as required
#export V= 99

all: pwm-build

libopencm3-build:
	$(CYGWIN_DIR)/bash -e ./compile.sh $(PWD)/libopencm3 V=1 all

libopencm3-clean:
	$(CYGWIN_DIR)/bash -e ./compile.sh $(PWD)/libopencm3 V=1 clean
	
pwm-build:
	@$(CYGWIN_DIR)/bash -c "mkdir -p $(OUTPUT_DIR) $(BUILD_DIR)"
	$(CYGWIN_DIR)/bash -e ./compile.sh $(PWD)/pwm V=1 $(OUTPUT_DIR)/$(PROJECT).bin $(OUTPUT_DIR)/$(PROJECT).list
	
pwm-clean:
	rm -rf ./build/$(PROJECT)
	$(CYGWIN_DIR)/bash -e ./compile.sh $(CPWD)/pwm V=1 clean

pwm-flash: pwm-build
	st-flash --reset write build/stm32-pwm/bin/stm32-pwm.bin 0x8000000
