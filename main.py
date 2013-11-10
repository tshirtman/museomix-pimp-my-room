# coding: utf-8
__version__ = '1.0'

from kivy.app import App

from kivy.uix.image import Image
from kivy.properties import DictProperty, StringProperty, ListProperty, ObjectProperty
from kivy.core.audio import SoundLoader
from kivy.uix.bubble import Bubble
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.togglebutton import ToggleButton
from kivy.uix.scatter import Scatter
from kivy.uix.behaviors import ButtonBehavior
from kivy.factory import Factory
from kivy.clock import Clock
from kivy.lib import osc
from os import listdir as oslistdir
from os.path import join, isdir
import socket

outSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
SERVER = '192.168.1.10'
PORT = 3000

PAINTED = 'tapisserie'
POSTER = 'affiche'
WINDOW = 'vidéo'
SOUND = 'son'
OBJECT = 'objet'

#PAINTED = 'images-fond'
#OBJECT = 'images-objets'

types = PAINTED, POSTER, WINDOW, SOUND, OBJECT


def osc_send(addr, data):
    outSocket.sendto(osc.createBinaryMsg(addr, data),  (SERVER, PORT))


def istype(choice, tp):
    return choice.split('/')[2].lower() == tp


def listdir(d):
    return (x for x in oslistdir(d) if isdir(join(d, x)))


class TypeButton(ToggleButton):
    t = StringProperty('')


class Poster(Scatter):
    source = StringProperty('')

    def __init__(self, **kw):
        super(Poster, self).__init__(**kw)
        self.bind(pos=self.send_update,
                  size=self.send_update,
                  rotation=self.send_update,
                  scale=self.send_update)

    def send_update(self, *args):
        return
        osc_send('/update',
                 [self.x, self,y, self.parent.children.index(self),
                  self.scale, self.rotation])

    def on_touch_up(self, touch):
        roomview = app.root.ids.roomview
        if self.parent == roomview.ids.dnd:
            roomview.ids.dnd.remove_widget(self)
            roomview.ids.posters.add_widget(self)

        return super(Poster, self).on_touch_up(touch)


class Choser(BoxLayout):
    choice = StringProperty('')

    def update_choice(self, *args):
        for i in self.children:
            if i.state == 'down':
                self.choice = i.t
                break
        else:
            self.choice = ''

    def on_choice(self, *args):
        if self.choice:
            choices = app.root.ids.choices
            choices.clear_widgets()
            for f in app.choices.get(self.choice, []):
                choices.add_widget(
                    Factory.Choice(source=join('data', 'resources', self.choice, f)))


class Choice(Image, ButtonBehavior):
    def manage(self, touch, *args):
        if self.collide_point(*touch.pos) or touch.grab_current == self:
           print "manage"
           app.manage_choice(touch, self.source)

    def on_touch_down(self, touch, *args):
        self.manage(touch)
        return super(Choice, self).on_touch_down(touch)


class PimpApp(App):
    choices = DictProperty({})
    category = StringProperty('')
    wallpaper = StringProperty('')
    ambiant = StringProperty('')
    ambiant_sound = ObjectProperty(None)

    def build(self):
        Clock.schedule_once(self.load_data, 0)
        return super(PimpApp, self).build()

    def load_data(self, *args):
        root = join('data', 'resources')
        choices = {}

        for d in listdir(root):
            choices[d] = {}
            for f in oslistdir(join(root, d)):
                choices[d][f] = join(root, d, f)

        self.choices = choices

    def on_wallpaper(self, *args):
        return
        osc_send('/wallpaper', self.wallpaper)

    def manage_choice(self, touch, choice):
        if istype(choice, PAINTED):
            self.wallpaper = choice

        elif istype(choice, POSTER):
            if 'poster' not in touch.ud:
                poster = Factory.Poster(
                        source=choice,
                        center=touch.pos,
                        size_hint=(None, None))
                self.root.ids.roomview.ids.dnd.add_widget(poster)
                touch.ud['poster'] = poster
                poster.on_touch_down(touch)
            else:
                touch.ud['poster'].center = touch.pos

        elif istype(choice, SOUND):
            self.ambiant = choice

    def on_ambiant(self, *args):
        if self.ambiant_sound:
            self.ambiant_sound.state = 'stop'
            self.ambiant_sound = None
        print self.ambiant
        self.ambiant_sound = SoundLoader.load(self.ambiant)
        self.ambiant_sound.state = 'play'


if __name__ == '__main__':
    app = PimpApp()
    app.run()

# /create  nom, x, y, z, scale, rotate
# /update nom, x, y, z, scale, rotate
# /wallpaper nom
# /delete id
# /snapshot
