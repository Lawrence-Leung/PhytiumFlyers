# user code
$(BUILD_OUT_PATH)/libuser.a: libuser.a
libuser.a: 
	$(call invoke_make_in_directory,board/user,makefile,all,USER_CSRC="$(USER_CSRC)" USER_ASRC="$(USER_ASRC)" USER_CXXSRC="$(USER_CXXSRC)" USER_INCLUDE_PATH="$(USER_INCLUDE)" LIBS_NAME=libuser EXTEND_CSRC="$(EXTEND_CSRC)" EXTEND_ASRC="$(EXTEND_ASRC)" EXTEND_CXXSRC="$(EXTEND_CXXSRC)")
libuser_debug:
	$(call invoke_make_in_directory,board/user,makefile,debug,USER_CSRC="$(USER_CSRC)" USER_ASRC="$(USER_ASRC)" USER_CXXSRC="$(USER_CXXSRC)" USER_INCLUDE_PATH="$(USER_INCLUDE)" EXTEND_CSRC="$(EXTEND_CSRC)" EXTEND_ASRC="$(EXTEND_ASRC)" EXTEND_CXXSRC="$(EXTEND_CXXSRC)")
libuser_info:
	$(call invoke_make_in_directory,board/user,makefile,compiler_info,USER_CSRC="$(USER_CSRC)" USER_ASRC="$(USER_ASRC)" USER_CXXSRC="$(USER_CXXSRC)" USER_INCLUDE_PATH="$(USER_INCLUDE)" EXTEND_CSRC="$(EXTEND_CSRC)" EXTEND_ASRC="$(EXTEND_ASRC)" EXTEND_CXXSRC="$(EXTEND_CXXSRC)")
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libuser.a

# board code
$(BUILD_OUT_PATH)/lib_board.a: lib_board.a
lib_board.a:
	$(call invoke_make_in_directory,board,makefile,all,)
lib_board_debug:
	$(call invoke_make_in_directory,board,makefile,debug,)
lib_board_info:
	$(call invoke_make_in_directory,board,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_board.a