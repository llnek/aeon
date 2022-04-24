.PHONY: clean All

All:
	@echo "----------Building project:[ aeon - Debug ]----------"
	@"$(MAKE)" -f  "aeon.mk"
clean:
	@echo "----------Cleaning project:[ aeon - Debug ]----------"
	@"$(MAKE)" -f  "aeon.mk" clean
