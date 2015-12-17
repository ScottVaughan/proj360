"""
A TCP echo server that handles multiple clients with polling.  Typing
Control-C will quit the server.
"""

import argparse

from poller import Poller

class Main:
    """ Parse command line options and perform the download. """
    def __init__(self):
        self.parse_arguments()

    def parse_arguments(self):
		''' parse arguments, which include '-p' for port '''
		parser = argparse.ArgumentParser(prog = 'web', description='parses command-line', add_help=True)
		parser.add_argument('-p', '--port',  type=int, default=8000, help='the port')
		parser.add_argument('-d', '--debug', help='debug', default=False)
		self.args = parser.parse_args()
		

    def run(self):
        p = Poller(self.args.port, self.args.debug)
        p.run()

if __name__ == "__main__":
    m = Main()
    m.parse_arguments()
    try:
        m.run()
    except KeyboardInterrupt:
        pass
