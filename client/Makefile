ifdef ComSpec
	RM=cmd /C del /F /Q
	RMDIR=cmd /C rd /S /Q
	MV=cmd /C move
	condiment=python -m condiment
	installer="\Program Files (x86)\Inno Setup 5\ISCC.exe" PimpMyRoom.iss
	cd client
else
	UNAME_S = $(shell uname -s)
	RM=rm -f
	RMDIR=rm -rf
	MV=mv
	condiment=condiment
	installer=tar -C dist -caf PimpMyRoom.tar.bz2 PimpMyRoom
	make_icon=tools/create_icon.sh
	ifeq ($(UNAME_S), Darwin)
		installer=hdiutil create dist/PimpMyRoom.dmg -srcfolder dist/PimpMyRoom.app -ov
		make_icon=tools/create_osx_icon.sh
	endif
endif

all: Package Installer

Package:
	-$(RMDIR) build
	-$(RMDIR) dist
	pyinstaller PimpMyRoom.spec -y

Installer:
	$(installer)

Restore:
	git reset --hard

Icon:
	$(make_icon)
