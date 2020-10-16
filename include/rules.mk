LINK=wcl.exe
LINK_OPTIONS=
RM=if exist $(1) del $(1)

.PRECIOUS: %.obj

%.com:
	$(call msg,$@,$^)
	$(LINK) $(LINK_OPTIONS) -fe=$@ $^
	
%.img: $(BOOTSECTOR)
	$(call msg,$@,$^)
	mkimg144.exe $(BOOTSECTOR:%=-bs %) -o $@ $(filter-out $(BOOTSECTOR),$^)

clean::
	$(call msg,$@,$^)
	$(call RM,*.com)
	$(call RM,*.obj)
	$(call RM,*.img)
	$(call RM,*.err)

.PHONY: clean
