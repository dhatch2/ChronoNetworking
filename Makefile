.PHONY: clean All

All:
	@echo "----------Building project:[ ChronoServer - Debug ]----------"
	@cd "ChronoServer" && "$(MAKE)" -f  "ChronoServer.mk"
clean:
	@echo "----------Cleaning project:[ ChronoServer - Debug ]----------"
	@cd "ChronoServer" && "$(MAKE)" -f  "ChronoServer.mk" clean
