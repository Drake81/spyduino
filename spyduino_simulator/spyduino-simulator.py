#!/usr/bin/env python

import subprocess
import time
import socket
import re
from threading import Thread
import tkinter as tk


class TCPClient(Thread):
    def __init__(self, host, port, device):
        Thread.__init__(self)
        self.host = host
        self.port = port
        self.device = device
        self.sim = None
        self.running = False
        self.connected = False
        self.sock = None

    def connect(self):
        self.running = True
        self.start()

    def reconnect(self):
        while not self.connected:
            try:
                self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.sock.connect((self.host, self.port))
                self.connected = True
                print("Connected")
            except Exception:
                pass

    def disconnect(self):
        self.connected = False
        self.running = False

    def run(self):
        self.reconnect()
        while self.running:
            if not self.sim:
                msgs = self.scrape_data()
                try:
                    self.sock.sendall(b'AT+CWLAP\n')
                    for msg in msgs:
                        self.sock.sendall(str.encode('+CWLAP:(5,"{}",{},"{}",{})\n'.format(msg[0], msg[1], msg[2], msg[3])))
                    self.sock.sendall(b'\nOK\n')
                except Exception as e:
                    print(e)
                    self.connected = False
                    print("Try Reconnect")
                    self.reconnect()
            else:
                try:
                    for msg in self.sim:
                        self.sock.sendall(b'AT+CWLAP')
                        self.sock.sendall(str.encode(msg))
                        time.sleep(2)
                except Exception as e:
                    print(e)
                    self.connected = False
                    print("Try Reconnect")
                    self.reconnect()
            time.sleep(1)
        self.sock.close()

    def scrape_data(self):
        clean_data = []
        raw_data = subprocess.Popen("iwlist " + self.device + " scanning", shell=True, stdout=subprocess.PIPE).stdout.read().decode("utf-8")
        raw_data_splitted = raw_data.split("Cell ")[1:]
        for raw_cell in raw_data_splitted:
            cell = raw_cell.split("Mode:")[0]
            # print(cell)
            cell_data = []

            m = re.search('.*Frequency:2.4.* GHz', cell)
            if m:
                m = re.search('.*ESSID:"(.*)".*', cell)
                if m:
                    cell_data.append(m.group(1))
                else:
                    cell_data.append("none")

                m = re.search('.* Signal level=(.\d+) dBm.*', cell)
                if m:
                    cell_data.append(m.group(1))
                else:
                    cell_data.append("none")

                m = re.search('.+ - Address: (.+).*', cell)
                if m:
                    cell_data.append(m.group(1))
                else:
                    cell_data.append("none")

                m = re.search('.*Channel:(\d+).*', cell)
                if m:
                    cell_data.append(m.group(1))
                else:
                    cell_data.append("none")
                clean_data.append(cell_data)
        # print(clean_data)
        return clean_data


class App(tk.Tk):
    def __init__(self, host, port, device, filename):
        tk.Tk.__init__(self)
        self.wm_title("Spyduino Simulator")
        self.protocol("WM_DELETE_WINDOW", self.on_close)
        self.host = host
        self.port = port
        self.device = device
        self.filename = filename
        self.TCPClient = TCPClient(self.host, self.port, self.device)
        # Define LEDs
        self.frame = tk.Frame(self, background="bisque")
        self.frame.pack(side="top", fill="both", expand=True)
        self.canvas = tk.Canvas(self.frame, bg="green")
        self.canvas.create_oval(10, 10, 40, 40, fill="red", tags="ledred")
        self.led_red_on = True
        self.canvas.create_oval(10, 50, 40, 80, fill="yellow", tags="ledyellow")
        self.led_yellow_on = False
        self.canvas.create_oval(10, 90, 40, 120, fill="#5EFF00", tags="ledgreen")
        self.led_green_on = False

        self.canvas.create_rectangle(20, 160, 60, 200, fill="grey", tags="s2")
        self.canvas.create_rectangle(20, 210, 60, 250, fill="grey", tags="s1")
        self.canvas.create_rectangle(280, 10, 320, 50, fill="grey", tags="rst")

        self.canvas.pack(side="top", fill="both", expand=True)
        # self.canvas.bind('<Button-1>', self.onCanvasClick)
        self.canvas.tag_bind('s1', '<Button-1>', self.on_S1)
        self.canvas.tag_bind('s2', '<Button-1>', self.on_S2)
        self.canvas.tag_bind('rst', '<Button-1>', self.on_RST)
        self.draw_light()
        print("Running...")

    # def onCanvasClick(self, event):
    #     print('Got canvas click')
    #     print(event.x, event.y, event.widget)
    #     print()

    def on_S1(self, event):
        if not self.TCPClient.connected:
            with open(self.filename, 'r') as f:
                self.TCPClient.sim = f.read().split("AT+CWLAP")[1:]
        self.on_S2(event)

    def on_S2(self, event):
        if not self.TCPClient.connected:
            self.wifi()
            print("Try Connect")
            self.TCPClient.connect()
        else:
            print("Try Close")
            self.on_RST(None)

    def on_RST(self, event):
        self.TCPClient.disconnect()
        self.TCPClient.join()
        self.led_yellow_on = False
        self.led_green_on = False
        self.led_red_on = True
        self.TCPClient = TCPClient(self.host, self.port, self.device)

    def wifi(self):
        print("WIFI Simulation")
        self.led_red_on = False
        self.canvas.itemconfig("ledred", fill="white")
        self.canvas.itemconfig("ledgreen", fill="#5EFF00")
        self.led_green_on = True

    def draw_light(self):
        if self.led_red_on:
            self.canvas.itemconfig("ledred", fill="red")
            self.light_on = False
        else:
            self.canvas.itemconfig("ledred", fill="white")
            self.light_on = True

        if self.TCPClient.connected:
            self.canvas.itemconfig("ledyellow", fill="yellow")
            self.light_on = False
        else:
            self.canvas.itemconfig("ledyellow", fill="white")
            self.light_on = True

        if self.led_green_on:
            self.canvas.itemconfig("ledgreen", fill="#5EFF00")
            self.light_on = False
        else:
            self.canvas.itemconfig("ledgreen", fill="white")
            self.light_on = True

        self.after(1000, self.draw_light)

    def on_close(self):
        self.TCPClient.disconnect()
        self.destroy()


HOST = "127.0.0.1"
PORT = 1337
#DEVICE = "wlp3s0"
DEVICE = "wlan0"
FILENAME = "simfile.txt"

app = App(HOST, PORT, DEVICE, FILENAME)
app.mainloop()
