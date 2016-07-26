.PHONY: clean All

All:
	@echo "----------Building project:[ BasicServer - Debug ]----------"
	@cd "BasicServer" && "$(MAKE)" -f  "BasicServer.mk"
clean:
	@echo "----------Cleaning project:[ BasicServer - Debug ]----------"
	@cd "BasicServer" && "$(MAKE)" -f  "BasicServer.mk" clean
