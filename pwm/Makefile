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

CFILES += $(PROJECT).c
# TODO - you will need to edit these two lines!
DEVICE=stm32f401cc
LIBNAME		= opencm3_stm32f4
DEFS		= -DSTM32F4
OOCD_FILE = board/stm32f4discovery.cfg

# You shouldn't have to edit anything below here.
VPATH += $(SHARED_DIR) $(sort $(dir $(CFILES)))

INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR=../libopencm3
OPENCM3_TEMPLATE_DIR=../libopencm3-template

include $(OPENCM3_DIR)/mk/genlink-config.mk
include $(OPENCM3_TEMPLATE_DIR)/rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk

$(OUTPUT_DIR)/$(PROJECT).elf: $(OBJS) $(LDSCRIPT) $(LIBDEPS)
	@printf "  LD\t$@\n"
	$(Q)$(LD) $(TGT_LDFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

