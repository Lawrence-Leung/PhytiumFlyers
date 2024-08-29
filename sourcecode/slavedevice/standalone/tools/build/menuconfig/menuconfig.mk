.PHONY: menuconfig oldconfig alldefconfig savedefconfig lddefconfig genconfig setconfig ldconfig
menuconfig:
	$(SDK_KCONFIG_DIR)/menuconfig.py
	$(SDK_KCONFIG_DIR)/genconfig.py

update_menuconfig:
	$(SDK_KCONFIG_DIR)/menuconfig_autosave.py
	$(SDK_KCONFIG_DIR)/genconfig.py


setconfig:
	$(SDK_KCONFIG_DIR)/setconfig.py $(SETCONFIG_ARG)
	$(SDK_KCONFIG_DIR)/genconfig.py

genconfig:
	$(SDK_KCONFIG_DIR)/genconfig.py

# backup current configs
oldconfig:
	$(SDK_KCONFIG_DIR)/oldconfig.py

# write configuration where all symbols and set as
#	default val
alldefconfig:
	$(SDK_KCONFIG_DIR)/alldefconfig.py

# # Saves a minimal configuration file that only lists symbols that differ in value
# #	from their defaults
savedefconfig:
	$(SDK_KCONFIG_DIR)/savedefconfig.py

lddefconfig:
	cp $(STANDALONE_DIR)/configs/$(DEF_KCONFIG) ./$(KCONFIG_CONFIG) -f
	@echo "get default configs at " $(STANDALONE_DIR)/configs/$(DEF_KCONFIG)

ldconfig:
	cp $(LDCONFIG_ARG) ./$(KCONFIG_CONFIG) -f
	@echo "get configs at " $(LDCONFIG_ARG)
	$(SDK_KCONFIG_DIR)/genconfig.py

