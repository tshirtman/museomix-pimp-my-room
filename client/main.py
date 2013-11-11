# coding: utf-8
__version__ = '1.0'

from kivy.app import App

from kivy.uix.image import Image
from kivy.properties import DictProperty, StringProperty, ListProperty, ObjectProperty, NumericProperty, BooleanProperty
from kivy.core.audio import SoundLoader
from kivy.uix.bubble import Bubble
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.togglebutton import ToggleButton
from kivy.uix.scatter import Scatter
from kivy.factory import Factory
from kivy.animation import Animation
from kivy.clock import Clock
from kivy.lib import osc
from os import listdir as oslistdir, sep, getcwd
from os.path import join, isdir, dirname
import socket

outSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
SERVER = '192.168.1.10'
PORT = 3000

PAINTED = 'fonds-vignettes'
POSTER = 'objets-vignettes'
VIDEO = 'video'
SOUND = 'son'
OBJECT = 'objets-vignettes'

#PAINTED = 'fond'
#OBJECT = 'objets'

MINIATURES = '-vignettes'

types = PAINTED, POSTER, VIDEO, SOUND, OBJECT


def osc_send(addr, data=None):
    try:
        outSocket.sendto(osc.createBinaryMsg(addr, data),  (SERVER, PORT))
    except:
        pass


def istype(choice, tp):
    return choice.split('/')[2].lower() == tp


def listdir(d):
    return (x for x in oslistdir(d) if isdir(join(d, x)))


class TypeButton(ToggleButton):
    t = StringProperty('')


class Poster(Scatter):
    source = StringProperty('')
    index = NumericProperty(0)

    def __init__(self, **kw):
        super(Poster, self).__init__(**kw)
        self.bind(pos=self.send_update,
                  size=self.send_update,
                  rotation=self.send_update,
                  scale=self.send_update)

    def send_update(self, *args):
        #print self.parent.children.index(self)
        #print [self.source.split(sep)[-1],
                  #self.x / self.parent.width,
                  #1 - self.y / self.parent.height,
                  #self.parent.children.index(self),
                  #self.scale, - self.rotation]

        if len(self.parent.children) > 1:
            self.index = max(x.index for x in self.parent.children if x is not self) + 1

        osc_send('/update',
                 [self.source.split(sep)[-1],
                  self.x / self.parent.width,
                  1 - self.y / self.parent.height,
                  self.index,
                  self.scale, - self.rotation])

    def on_touch_down(self, touch, *args):
        #if touch.is_double_tap:
            #self.ids.close.disabled = False
            #Animation(opacity=1, d=.1).start(self.ids.close)
        return super(Poster, self).on_touch_down(touch, *args)

    def on_touch_up(self, touch):
        roomview = app.root.ids.roomview
        #if self.parent == roomview.ids.dnd:
            #roomview.ids.dnd.remove_widget(self)
            #roomview.ids.posters.add_widget(self)

        #if not  self.ids.close.disabled:
            #a = Animation(d=5)
            #a &= Animation(opacity=0, d=2)
            #a.bind(on_complete=lambda *x:
                               #self.ids.close.setter('disabled')(self.ids.close, True))
            #a.start(self.ids.close)

        return super(Poster, self).on_touch_up(touch)


class VideoPoster(Poster):
    pass


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
                if isdir(f):
                    continue
                #if istype(f, VIDEO):
                    #c = Factory.VideoChoice()
                #else:
                c = Factory.Choice(source=join(dirname(__file__), 'data', 'resources', self.choice, f))
                choices.add_widget(c)


class Choice(Image):
    disabled = BooleanProperty(False)

    def manage(self, touch, *args):
        if self.collide_point(*touch.pos):
           print "manage"
           #app.manage_choice(touch, self.source.replace(MINIATURES, ''))
           app.manage_choice(touch, self.source)

    def on_touch_down(self, touch, *args):
        if not self.disabled:
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
        root = join(dirname(__file__), 'data', 'resources')
        choices = {}

        for d in listdir(root):
            choices[d] = {}
            for f in oslistdir(join(root, d)):
                choices[d][f] = join(root, d, f)

        self.choices = choices

    def on_wallpaper(self, *args):
        print "sending wallpaper"
        osc_send('/wallpaper', [self.wallpaper.split(sep)[-1], ])

    def manage_choice(self, touch, choice):
        if istype(choice, PAINTED):
            self.wallpaper = choice.replace(MINIATURES, '')

        elif istype(choice, POSTER):
            if 'poster' not in touch.ud:
                print "loading %s"
                poster = Factory.Poster(
                        #scale=0.1,
                        source=choice,
                        center=touch.pos,
                        size_hint=(None, None))
                self.root.ids.roomview.ids.posters.add_widget(poster)
                touch.ud['poster'] = poster
                #widgets = [x for x in app.root.ids.choices.children if x.source == choice]
                #if widgets:
                    #widgets[0].disabled = True

                poster.on_touch_down(touch)
            else:
                touch.ud['poster'].center = touch.pos

        #elif istype(choice, VIDEO):
            #if 'poster' not in touch.ud:
                #poster = Factory.VideoPoster(
                        #source=choice,
                        #center=touch.pos,
                        #size_hint=(None, None))
                #self.root.ids.roomview.ids.dnd.add_widget(poster)
                #touch.ud['poster'] = poster
            #else:
                #touch.ud['poster'].center = touch.pos

        elif istype(choice, SOUND):
            self.ambiant = choice

    def on_ambiant(self, *args):
        if self.ambiant_sound:
            self.ambiant_sound.state = 'stop'
            self.ambiant_sound = None
        print self.ambiant
        self.ambiant_sound = SoundLoader.load(self.ambiant)
        if self.ambiant_sound:
            self.ambiant_sound.state = 'play'
        else:
            print 'error loading %s' % self.ambiant

    def clear(self, *args):
        osc_send('/clearall', [])
        self.wallpaper = ''
        self.root.ids.roomview.ids.posters.clear_widgets()
        self.root.ids.roomview.ids.windows.clear_widgets()


if __name__ == '__main__':
    app = PimpApp()
    app.run()

# /create  nom, x, y, z, scale, rotate
# /update nom, x, y, z, scale, rotate
# /wallpaper nom
# /delete id
#=None /snapshot
