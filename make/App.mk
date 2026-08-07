# Shared development workflow for one application. The including Makefile sets
# APP, APP_CMAKE_OPTION, and OTHER_APP_CMAKE_OPTIONS.

APP_BUILD_DIR := $(PROJECT_ROOT)/build/$(APP)
APP_BINARY_DIR := $(APP_BUILD_DIR)/apps/$(APP)

ifeq ($(shell uname -s),Darwin)
APP_BUNDLE := $(APP_BINARY_DIR)/$(APP).app
APP_EXECUTABLE := $(APP_BUNDLE)/Contents/MacOS/$(APP)
else
APP_EXECUTABLE := $(APP_BINARY_DIR)/$(APP)
endif

.DEFAULT_GOAL := help

.PHONY: help all dev configure build rebuild run run-fg package icons clean

help: ## Show this help (default target)
	@awk -v app="$(APP)" 'BEGIN {FS = ":.*##"} \
	  /^##@/ {section = substr($$0, 5); gsub(/\$$\(APP\)/, app, section); \
	           printf "\n\033[1m%s\033[0m\n", section} \
	  /^[a-zA-Z0-9_-]+:.*##/ {description = $$2; gsub(/\$$\(APP\)/, app, description); \
	           printf "  \033[36m%-12s\033[0m %s\n", $$1, description}' $(MAKEFILE_LIST)
	@echo ""

##@ $(APP)
all: ## Clean, build, and run $(APP) in the foreground
	$(MAKE) clean
	$(MAKE) build
	$(MAKE) run-fg

dev: ## Configure, incrementally build, and run $(APP)
	$(MAKE) build
	$(MAKE) run-fg

configure: ## Configure an isolated build containing only $(APP)
	cmake -S "$(PROJECT_ROOT)" -B "$(APP_BUILD_DIR)" -G Ninja \
		-DCMAKE_BUILD_TYPE=Debug -D$(APP_CMAKE_OPTION)=ON $(OTHER_APP_CMAKE_OPTIONS)

build: configure ## Build $(APP) and only its required dependencies
	cmake --build "$(APP_BUILD_DIR)" --target "$(APP)"

rebuild: ## Rebuild $(APP) without explicitly reconfiguring
	cmake --build "$(APP_BUILD_DIR)" --target "$(APP)"

run: ## Launch $(APP)
ifeq ($(shell uname -s),Darwin)
	open "$(APP_BUNDLE)"
else
	"$(APP_EXECUTABLE)"
endif

run-fg: ## Run $(APP) in the foreground
	"$(APP_EXECUTABLE)"

package: build ## Package only $(APP) with CPack
	cd "$(APP_BUILD_DIR)" && cpack

icons: ## Generate every platform icon from the app's master SVG
	"$(PROJECT_ROOT)/scripts/generate-icons.sh" "$(APP)" --name "$(APP_DISPLAY_NAME)"

clean: ## Remove only $(APP)'s isolated build directory
	rm -rf "$(APP_BUILD_DIR)"
