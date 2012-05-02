from mpdpp import Client

import sys

if __name__ == "__main__":
    client = Client()
    client.ping()

    if len(sys.argv) != 2:
        print "Usage: test.py <command>"
        sys.exit(1)

    command = sys.argv[1]

    if command == "status":
        status = client.status()

        print "playing:", status.playing
        print "decoder_ready:", status.decoder_ready
        print "decoder_requested_frames:", status.decoder_requested_frames

    elif command == "play":
        client.play()
    elif command == "pause":
        client.pause()
    else:
        print "Invalid command"
        sys.exit(1)
