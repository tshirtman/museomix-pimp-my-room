import os
from os.path import join

IS_LINUX = os.name == 'posix' and os.uname()[0] == 'Linux'
if IS_LINUX:
    from PyInstaller.depend import dylib
    dylib._unix_excludes.update({
        r'.*nvidia.*': 1,
        })

    dylib.exclude_list = dylib.ExcludeList()

from kivy.tools.packaging.pyinstaller_hooks import install_hooks
from os.path import expanduser
install_hooks(globals())

a = Analysis(['main.py'],
             pathex=['client/'],
             hiddenimports=['numpy.core.multiarray'],
             excludes=['gobject', 'gio', 'PIL', 'gst', 'gtk', 'gi', 'wx', 'twisted', 'curses'] + (['pygame'] if IS_LINUX else []),
             runtime_hooks=None)

pyz = PYZ(a.pure)

name = 'PympMyRoom%s' % ('.exe' if os.name == 'nt' else '')

exe = EXE(pyz,
          a.scripts,
          exclude_binaries=True,
          name=name,
          debug=False,
          strip=None,
          upx=True,
          console=False,
          icon=join('data', 'icon_PympMyRoom.ico'))
coll = COLLECT(exe,
               Tree('client/',
                    excludes=[
                        '.git', '*.spec', '*.ini',
                        'Makefile', 'build', 'dist', '*.pyo',
                        '*.pyc', 'setup.py', '*.iss', 'installer',
                        '.gitignore', '*.spw', '*.swo', '*.swn',
                        'tools',  '*.key',
                        ]),
               a.binaries,
               a.zipfiles,
               a.datas,
               strip=None,
               upx=True,
               name='PympMyRoom')

app = BUNDLE(coll,
             name='PympMyRoom.app',
             icon='data/PympMyRoom.icns')
