from mom.client import Client

def action_kill(opts):
    """Kill the running daemon"""
    with Client() as client:
        client.ping()
        client.kill()

def action_play(opts):
    """Resume playback"""
    with Client() as client:
        client.ping()
        client.play()

def action_pause(opts):
    """Pause playback"""
    with Client() as client:
        client.ping()
        client.pause()

def action_status(opts):
    """Pause playback"""
    with Client() as client:
        client.ping()
        status = client.status()

    print u"Playing:", status.playing
    print u"Decoder Ready:", status.decoder_ready
    print u"Decoder Requested Frames:", status.output_pending_frames

actions = dict()
actions["kill"] = action_kill
actions["play"] = action_play
actions["pause"] = action_pause
actions["status"] = action_status

def print_help(program_name="mpdc"):
    print u"Usage: {0} <action>".format(program_name)
    print u"Available Actions:"

    for name, action in actions.items():
        short = action.__doc__.strip().split("\n", 2)[0]
        print u"  {0}: {1}".format(name, short)

def main(program_name, args):
    if len(args) < 1:
        print_help(program_name)
        return 1

    action_name = args[0]

    action_function = actions.get(action_name.lower(), None)

    if action_function is None:
        print u"Invalid Action:", action_name
        print_help(program_name)
        return 1

    action_function(args[1:])
