.PHONY: clean build format run help format appimage flatpak windows windows-installer
.DEFAULT_GOAL := help

VERSION := $(shell sed -n 's/^set(RELEASE_SERVICE_VERSION_\(MAJOR\|MINOR\|MICRO\) "\(.*\)")$$/\2/p' CMakeLists.txt | paste -sd.)
FLATPAK_MANIFEST := packaging/flatpak/com.someblocks.d2rloader.yml
FLATPAK_BUILDER := flatpak run org.flatpak.Builder

help: ## Displays this help message.
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-15s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

prepare:
	cmake -B build/

build:
	cmake --build build/ -j 6

run:
	./build/bin/d2rloader

format:
	git clang-format --staged --extensions 'cpp,h,hpp,c'

appimage: ## Build the AppImage into build-appimage/.
	packaging/appimage/build-appimage.sh

flatpak: ## Build the Flatpak bundle into the project root.
	$(FLATPAK_BUILDER) --user --install-deps-from=flathub --force-clean --repo=repo build-flatpak $(FLATPAK_MANIFEST)
	flatpak build-bundle --runtime-repo=https://dl.flathub.org/repo/flathub.flatpakrepo \
		repo D2RLoader-$(VERSION)-x86_64.flatpak com.someblocks.d2rloader

windows: ## Build the portable Windows zip into build-windows/ (MSYS2 UCRT64).
	packaging/windows/build-zip.sh

windows-installer: windows ## Build the Windows installer into build-windows/ (MSYS2 UCRT64).
	packaging/windows/build-installer.sh

clean: ## Remove unwanted stuff such as __pycache__, etc...
	find . -name '*.pyc' -exec rm -f {} +
	find . -name '*.pyo' -exec rm -f {} +
	find . -name '*~' -exec rm -f {} +
	find . -name '__pycache__' -exec rm -rf {} +
	find . -name '.ruff_cache' -exec rm -rf {} +
