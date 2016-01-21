# The Homebrew Launcher

#### Presentation

The Homebrew Launcher (hbmenu for short) is the default menu for ninjhax, a 3DS homebrew-enabling exploit released on November 20th 2014. It is a fairly simple (and beautiful) menu that lists homebrew applications and lets you run them.

#### Usage

To use hbmenu as your ninjhax menu, simply rename the 3dsx executable to boot.3dsx and place it at the root of your SD card.

Press START in hbmenu to reboot your console into home menu. Use the D-PAD, CIRCLE-PAD or the touchscreen to select an application, and press A or touch it again to start it.

hbmenu starts in the sdmc:/3ds/ directory for applications; it will recognise folders containing "boot.3dsx" or "\[folder name\].3dsx" as "application bundles", other folders can be opened and browsed as you'd expect. You can have an icon file named "icon.bin", "icon.smdh", "icon.icn", "\[folder name\].smdh" or "\[folder name\].icn". hbmenu will also recognize stray 3dsx executables located in the sdmc:/3ds/ directory.

Here is an example directory structure that hbmenu will have no trouble recognizing :

- sdmc:/
  - 3ds/
    - 3dscraft/
      - 3dscraft.3dsx
      - icon.bin
    - blargsnes/
      - boot.3dsx
      - blargsnes.smdh
    - gameyob/
      - gameyob.3dsx
      - gameyob.icn
    - cubedemo/
      - boot.3dsx
      - icon.icn
    - 3dnes.3dsx
    - ftpony.3dsx

If hbmenu does not find an icon file to associate with a given 3dsx, it will display a default icon and the path to the executable instead of the actual metadata for that executable.

Hbmenu also allows you to create "shortcuts" which are xml files containing a path to a 3dsx file and optional arguments to pass to the .3dsx. This file can also include a path to icon data as well as name, description and author text using tags as follows :-

<shortcut>

  - <executable>The path to the 3dsx file goes here.</executable>
  - <icon>path to smdh icon data</icon>
  - <arg>Place arguments to be passed to 3dsx here.</arg>
  - <name>Name to display</name>
  - <description>Description of homebrew app</description>
  - <author>Name of the author</author>

</shortcut>

Arguments are space or tab separated but can use single or double quotes to contain whitespace.

Name, description and author will be read from the .3dsx if it has embedded smdh data or from the supplied icon path. The fields in the xml file will then override their respective entries.

Note that while you can hotswap the SD card while hbmenu is running and it *should* work fine, in practice this feature has proven to be unstable, so use at your own risk. It is recommended that you instead use a file transfer homebrew application such as ftpony to transfer files without rebooting.
Note that while you can hotswap the SD card while hbmenu is running and it *should* work fine, in practice this feature has proven to be unstable, so use at your own risk. It is recommended that you instead use a file transfer homebrew application such as ftpony to transfer files without rebooting.

#### Technical notes

Currently, everything in hbmenu is rendered in software. The reason for that is of course that homebrew GPU support is still unstable and very much a work in progress. Eventually, one of our goals should be to make hbmenu use the GPU for rendering.

hbmenu uses some funky service commands to launch 3dsx files. If you're interested in launching 3dsx files from your own application, you should look here.

#### Netloader

The netloader has now been replaced with 3dslink. Press Y to activate as usual then run 3dslink <3dsxfile> if your network can cope with UDP broadcast messages.
If 3dslink says 3DS not found then you can use -a <ip address> to tell it where to send the file.

All the other arguments you give 3dslink will be passed as arguments to the launched 3dsx file. You can also specify argv[0] with -0 <argument> which is useful for
setting the current working directory if you already have data files in a particular place i.e. 3dslink myfile.3dsx -0 sdmc:/3ds/mydata/

3dslink is provided with devkitARM or you can download binaries from http://davejmurphy.com/3dslink/

#### Building

3dslink uses zlib for compression so you'll need to compile and install zlib for 3DS. You can do this from a bash shell (use the devkitPro provided msys bash on windows)

    export PORTLIBS        := $(DEVKITPRO)/portlibs/armv6k
    export PATH            := $(DEVKITARM)/bin:$(PATH)
    export PKG_CONFIG_PATH := $(PORTLIBS)/lib/pkgconfig
    export CFLAGS          := -march=armv6k -mtune=mpcore -mfloat-abi=hard -O3 -mword-relocations
    export CPPFLAGS        := -I$(PORTLIBS)/include
    export LDFLAGS         := -L$(PORTLIBS)/lib

    CHOST=arm-none-eabi ./configure --static --prefix=$(PORTLIBS)
    make && make install

Binaries of 3ds_hb_menu can be downloaded from https://github.com/smealum/3ds_hb_menu/releases

#### Contributing

hbmenu is looking for contributors ! We're making this repository public so that you, the community, can make hbmenu into the menu of your dreams. Or show you how to make your own, better menu ! Of course we'd rather you improved hbmenu rather than went off and started fragmenting the userbase, but any contributions to the homebrew scene are welcome. Feel free to use code from hbmenu for your own projects, so long as you give credit to its original authors.

#### Credits

- smea : code
- GEMISIS : code
- fincs : code
- mtheall : code
- Fluto : graphics
- Arkhandar : graphics
- dotjasp : graphics (regionfree icon)
