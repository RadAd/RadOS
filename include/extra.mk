empty:=
escape:=
space:=$(empty) $(empty)
msg=@echo $(color_target)*** $(1): $(2)$(color_reset)

ifdef MAKE_TERMOUT
	COLOR=y
endif

ifeq ($(COLOR), y)
  color_target=$(escape)[34m
  color_reset=$(escape)[0m
endif
