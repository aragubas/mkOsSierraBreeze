# mkOS Sierra Breeze

A fork of [Sierra Breeze Enhanced](https://github.com/kupiqu/SierraBreezeEnhanced) with some changes and simplified

## Overview

Features:

 * Button spacing and padding Options.
 * Option to make all button symbols to appear at unison on hovering (Note: it does not apply to symbol themes).
 * Titlebar style options: SBE own style of Line Separation between Titlebar and Window / Match Titlebar color to Window color / Hide Titlebar under certain circumstances (Never/Maximization/Any Maximization (including H/V)/Always) / Gradient Adjustments / Opacity Adjustments.
 * Specific Shadow settings for inactive windows


## Installation

Please note that after installing, you need to restart KWin by executing either `kwin_x11 --replace` or `kwin_wayland --replace` in krunner (depending on whether your session runs upon X11 or Wayland). Alternatively, restarting the KDE session is obviously also an option. Then, mkOS Sierra Breeze will appear in *System Settings &rarr; Application Style &rarr; Window Decorations*.

### Method 1: Compile from source code
*Compilation should not be done against versions of KWin < 5.14.*

#### Step 1: Build dependencies
- Ubuntu
``` shell
sudo apt install build-essential libkf5config-dev libkdecorations2-dev libqt5x11extras5-dev qtdeclarative5-dev extra-cmake-modules libkf5guiaddons-dev libkf5configwidgets-dev libkf5windowsystem-dev libkf5coreaddons-dev libkf5iconthemes-dev gettext cmake
```
- Arch Linux
``` shell
sudo pacman -S base-devel # Required development tools
sudo pacman -S kdecoration qt5-declarative qt5-x11extras    # Decoration
sudo pacman -S cmake extra-cmake-modules                    # Installation
```
- Fedora
``` shell
sudo dnf install cmake extra-cmake-modules
sudo dnf install "cmake(Qt5Core)" "cmake(Qt5Gui)" "cmake(Qt5DBus)" "cmake(Qt5X11Extras)" "cmake(KF5GuiAddons)" "cmake(KF5WindowSystem)" "cmake(KF5I18n)" "cmake(KDecoration2)" "cmake(KF5CoreAddons)" "cmake(KF5ConfigWidgets)"
```

- Alpine Linux
``` shell
sudo apk add extra-cmake-modules qt5-qtbase-dev kdecoration-dev kcoreaddons-dev kguiaddons-dev kconfigwidgets-dev kwindowsystem-dev ki18n-dev kiconthemes-dev
```

#### Step 2: Then compile and install
- Install from script:
```sh
chmod +x install.sh
./install.sh
```
- Or more manually:
Open a terminal inside the source directory and do:
```sh
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DKDE_INSTALL_LIBDIR=lib -DBUILD_TESTING=OFF -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
make
sudo make install
```


## Uninstall

- Method 1: Use your Package manager
- Method 2: Run the uninstall script
```sh
chmod +x uninstall.sh
./uninstall.sh
```
- Method 3: or manually if previously ran the install script
```sh
cd build
sudo make uninstall
```


## Credits
Sierra Breeze Enhanced, Breeze, Sierra Breeze and Breeze Enhanced for obvious reasons :)
