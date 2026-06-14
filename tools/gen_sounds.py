#!/usr/bin/env python3
"""Generate retro-style sound effects for arkanoid as 16-bit mono WAV files.

No external dependencies — uses only the standard library. Run from the repo
root; writes into build/sounds/ next to the other runtime assets.

    python3 tools/gen_sounds.py
"""
import math
import os
import struct
import wave

RATE = 44100
OUT_DIR = os.path.join("build", "sounds")


def envelope(i, n, attack=0.01, release=0.3):
    """Simple attack/release amplitude envelope in [0, 1]."""
    t = i / n
    a = attack
    r = release
    if t < a:
        return t / a
    if t > 1.0 - r:
        return max(0.0, (1.0 - t) / r)
    return 1.0


def square(freq, t):
    return 1.0 if math.sin(2 * math.pi * freq * t) >= 0 else -1.0


def tone(freq, dur, vol=0.5, wave_fn=square, attack=0.01, release=0.3):
    n = int(RATE * dur)
    return [vol * envelope(i, n, attack, release) * wave_fn(freq, i / RATE)
            for i in range(n)]


def glide(f0, f1, dur, vol=0.5):
    n = int(RATE * dur)
    out = []
    phase = 0.0
    for i in range(n):
        f = f0 + (f1 - f0) * (i / n)
        phase += 2 * math.pi * f / RATE
        s = 1.0 if math.sin(phase) >= 0 else -1.0
        out.append(vol * envelope(i, n) * s)
    return out


def noise_burst(dur, vol=0.5):
    import random
    random.seed(1)
    n = int(RATE * dur)
    return [vol * envelope(i, n, 0.005, 0.6) * (random.random() * 2 - 1)
            for i in range(n)]


def mix(*tracks):
    n = max(len(t) for t in tracks)
    out = [0.0] * n
    for t in tracks:
        for i, s in enumerate(t):
            out[i] += s
    return out


def seq(*tracks):
    out = []
    for t in tracks:
        out.extend(t)
    return out


def write_wav(name, samples):
    path = os.path.join(OUT_DIR, name)
    with wave.open(path, "w") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        frames = bytearray()
        for s in samples:
            v = int(max(-1.0, min(1.0, s)) * 32767)
            frames += struct.pack("<h", v)
        w.writeframes(bytes(frames))
    print("wrote", path)


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    # Retro power-on jingle: rising arpeggio with a high flourish
    write_wav("start.wav",
              seq(tone(392, 0.09, 0.4), tone(523, 0.09, 0.4),
                  tone(659, 0.09, 0.4), tone(784, 0.09, 0.4),
                  tone(1046, 0.10, 0.45), tone(1318, 0.20, 0.45)))
    write_wav("paddle.wav", tone(440, 0.07, 0.5, release=0.5))
    write_wav("wall.wav", tone(700, 0.04, 0.4, release=0.5))
    write_wav("hit.wav", tone(300, 0.06, 0.45, release=0.5))
    write_wav("destroy.wav",
              mix(noise_burst(0.25, 0.35), glide(400, 90, 0.25, 0.3)))
    write_wav("powerup.wav",
              seq(tone(523, 0.06, 0.4), tone(659, 0.06, 0.4),
                  tone(784, 0.08, 0.4)))
    write_wav("life_lost.wav", glide(440, 150, 0.45, 0.45))
    write_wav("level.wav",
              seq(tone(523, 0.08, 0.4), tone(659, 0.08, 0.4),
                  tone(784, 0.08, 0.4), tone(1046, 0.14, 0.45)))
    write_wav("gameover.wav",
              seq(tone(392, 0.15, 0.45), tone(330, 0.15, 0.45),
                  tone(262, 0.30, 0.45)))


if __name__ == "__main__":
    main()
