# Make settings for integrating Inform's user interface and core software.
# This file contains only those settings likely to differ on different
# platforms, and the idea is that each user interface maintainer will keep
# his or her own version of this file.

# This is make-integration-settings.mk for Windows.

INTEGRATION = TRUE

# Now, the locations to which resources must be copied, inside the
# application. These pathnames mustn't contain spaces:

BUILTINCOMPS = ../../Build/Compilers
INTERNAL = ../../Build/Internal
BUILTINHTML = ../../Build/Documentation
BUILTINHTMLINNER = ../../Build/Documentation/sections

# Various executables are copied into the BUILTINCOMPS folder, but their
# filenames when copied there have sometimes differed between platforms.

INBLORBNAME = inblorb
INFORM6NAME = inform6
INFORM7NAME = inform7
INTESTNAME = intest

# Indoc options for generating the inside-the-application documentation:

INDOCOPTS = windows_app

# For reasons to do with CSS, the following should be "-no-font" for Windows:

INRTPSOPTS = -no-font