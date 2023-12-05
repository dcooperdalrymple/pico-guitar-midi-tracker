import time
import board
import busio
import adafruit_midi
import digitalio

from adafruit_midi.note_off import NoteOff
from adafruit_midi.note_on import NoteOn

uart = busio.UART(board.GP4, board.GP5, baudrate=31250, timeout=0.001)
midi = adafruit_midi.MIDI(
    midi_out=uart,
    out_channel=0
)

led = digitalio.DigitalInOut(board.GP2)
led.direction = digitalio.Direction.OUTPUT

print("MIDI Output Test")
print("Channel:", midi.out_channel + 1)

while True:
    led.value = True
    midi.send(NoteOn(48, 20))
    time.sleep(0.75)
    led.value = False
    midi.send(NoteOff(48, 0))
    time.sleep(0.25)
