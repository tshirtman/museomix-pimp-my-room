from kivy.lib import osc
import socket

outSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
SERVER = '192.168.1.10'
PORT = 3000

for i in range(100):
    #osc.sendMsg("/create", [], SERVER, PORT)
    outSocket.sendto(osc.createBinaryMsg('/create', [123.456, ]),  (SERVER, PORT))
