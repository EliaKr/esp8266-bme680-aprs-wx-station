# A python script to send your aprs message to APRS-IS. Fill in your information in lines 19, 38.

import socket
import time
import random

def send_aprs_message(aprs_message):
    server = 'rotate.aprs2.net'
    port = 14580

    # Connect to the APRS server
    print(f"Connecting to APRS server: {server}:{port}")
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((server, port))

    # Log in to the APRS server
    # Fill in your details here
    print("Logging in to APRS server")
    login_command = f'user USER pass PASS vers PythonAPRS 1.0\n'
    s.send(login_command.encode('utf-8'))

    # Send the APRS message
    print(f"Sending APRS message: {aprs_message}")
    s.send((aprs_message + '\n').encode('utf-8'))
    print("APRS message sent")

    # Close the connection
    s.close()

if __name__ == "__main__":
    for i in range(1, 10):

        temp = random.randint(34, 64)
        humidity = random.randint(34, 74)
        pressure = random.randint(10100, 10300)

        # Add your message here
        aprs_message = f"YOUR_MESSAGE"
        send_aprs_message(aprs_message)
        time.sleep(120)
