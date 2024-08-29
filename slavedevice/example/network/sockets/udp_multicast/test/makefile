
	

all:
	gcc multicast_server_ipv4.c -o multicast_server_ipv4.out
	gcc multicast_server_ipv6.c -o multicast_server_ipv6.out
	cp multicast_server_ipv4.out ../
	cp multicast_server_ipv6.out ../
	


.PHONY: clean

RM_FILES = $(foreach file,$(1),rm -f $(file)$(EOL))

clean:
	$(call RM_DIRS,$(OUTPUT_DIR))
	$(call RM_FILES,*.elf)
	$(call RM_FILES,*.bin)
	$(call RM_FILES,*.dis)
	$(call RM_FILES,*.map)
	$(call RM_FILES,*.tar.gz)
	$(call RM_FILES,*.asm)
	$(call RM_FILES,*.out)
