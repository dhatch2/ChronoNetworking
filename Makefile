.PHONY: clean All

All:
	@echo "----------Building project:[ BasicVehicleServer - Debug ]----------"
	@cd "BasicVehicleServer" && "$(MAKE)" -f  "BasicVehicleServer.mk"
clean:
	@echo "----------Cleaning project:[ BasicVehicleServer - Debug ]----------"
	@cd "BasicVehicleServer" && "$(MAKE)" -f  "BasicVehicleServer.mk" clean
