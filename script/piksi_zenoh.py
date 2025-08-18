import zenoh
import json
import sys

# Zenoh configuration for UDP transport
conf = zenoh.Config()
conf.insert_json5('mode', '"peer"')
conf.insert_json5('listen/endpoints', '["udp/0.0.0.0:7447"]')

session = zenoh.open(conf)

def listener(sample):
    try:
        data = json.loads(sample.payload.to_string())
        print("Received GPS data:")
        print(data)
    except Exception as e:
        print(f"Error decoding payload: {e}")

sub = session.declare_subscriber('fdcl/piksi', listener)

print("Subscribed to 'fdcl/piksi'. Press Ctrl+C to exit.")
sys.stdin.readline()

sub.undeclare()
session.close()