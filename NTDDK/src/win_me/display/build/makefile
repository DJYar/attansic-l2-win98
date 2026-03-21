all:
	@echo Building i910_testvxd miniVDD and user tool
	@cd ..\minivdd\i910test 
	@$(MAKE)
	@cd ..\user\vxd_ping 
	@$(MAKE)

clean:
	@cd ..\minivdd\i910test 
	@$(MAKE) clean
	@cd ..\user\vxd_ping 
	@$(MAKE) clean
