.PHONY: clean All

All:
	@echo "----------Building project:[ Foundation - Debug ]----------"
	@cd "/home/tristan/github/tacent/Modules/Foundation/Linux" && "$(MAKE)" -f  "Foundation.mk"
	@echo "----------Building project:[ Math - Debug ]----------"
	@cd "/home/tristan/github/tacent/Modules/Math/Linux" && "$(MAKE)" -f  "Math.mk"
	@echo "----------Building project:[ System - Debug ]----------"
	@cd "/home/tristan/github/tacent/Modules/System/Linux" && "$(MAKE)" -f  "System.mk"
	@echo "----------Building project:[ Image - Debug ]----------"
	@cd "/home/tristan/github/tacent/Modules/Image/Linux" && "$(MAKE)" -f  "Image.mk"
	@echo "----------Building project:[ UnitTests - Debug ]----------"
	@"$(MAKE)" -f  "UnitTests.mk" && "$(MAKE)" -f  "UnitTests.mk" PostBuild
clean:
	@echo "----------Cleaning project:[ Foundation - Debug ]----------"
	@cd "/home/tristan/github/tacent/Modules/Foundation/Linux" && "$(MAKE)" -f  "Foundation.mk"  clean
	@echo "----------Cleaning project:[ Math - Debug ]----------"
	@cd "/home/tristan/github/tacent/Modules/Math/Linux" && "$(MAKE)" -f  "Math.mk"  clean
	@echo "----------Cleaning project:[ System - Debug ]----------"
	@cd "/home/tristan/github/tacent/Modules/System/Linux" && "$(MAKE)" -f  "System.mk"  clean
	@echo "----------Cleaning project:[ Image - Debug ]----------"
	@cd "/home/tristan/github/tacent/Modules/Image/Linux" && "$(MAKE)" -f  "Image.mk"  clean
	@echo "----------Cleaning project:[ UnitTests - Debug ]----------"
	@"$(MAKE)" -f  "UnitTests.mk" clean
