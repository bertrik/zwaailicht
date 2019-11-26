#!/usr/bin/env python3

"""
   Polls a server for ticket status over HTTP and
   pushes the result over serial to a toy blue light
"""

import time
import argparse
import requests
import serial

class FrontendPoller():
    """ Polls ticket status from HTTP, pushes a command over serial """

    def __init__(self, url, serialport, interval):
        self.url = url
        self.serialport = serialport
        self.interval = interval

    def get_result(self, text, phrase):
        return phrase in text

    def run(self):
        """ runs the main process of this class """
        with serial.Serial(self.serialport, 115200) as ser:
            while True:
                # get the html content
                print('Polling ticket status from {}'.format(self.url))
                response = requests.get(self.url)
                print('Response: {}'.format(response.status_code))

                # scrape for specific color indicating a problem
                result = (response.status_code == 200) and self.get_result(response.text, 'background-color:#80b6ed')
                print('Found blue ticket: {}'.format(result))
                if result:
                    cmd = 'turn 3000\n'
                    print('> send serial command: {}'.format(cmd.strip()))
                    ser.write(cmd.encode('ascii'))

                # wait until next poll
                print('> sleeping {} seconds until next poll'.format(self.interval))
                time.sleep(self.interval)

def main():
    """ The main entry point """

    parser = argparse.ArgumentParser()
    parser.add_argument("-u", "--url", help="The service frontend URL",
                        default="http://technet/service-frontend-mm")
    parser.add_argument("-s", "--serialport", help="The serial port to write to", default="/dev/ttyUSB0")
    parser.add_argument("-i", "--interval", help="The poll interval (seconds)", default=60)
    args = parser.parse_args()

    poller = FrontendPoller(args.url, args.serialport, args.interval)
    poller.run()

if __name__ == "__main__":
    main()
