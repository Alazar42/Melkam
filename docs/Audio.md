# MelkamEngine Audio Documentation

MelkamEngine provides a high-performance **Audio Subsystem** (`Audio` & `Sound`) built on top of **SDL3**.

Include header:
```cpp
#include "audio/Audio.hpp"
```

---

## 1. Quick Start

```cpp
#include "audio/Audio.hpp"

int main() {
    // 1. Initialize audio subsystem
    Audio::init();

    // 2. Play a sound effect (.wav)
    Audio::play("assets/laser.wav", 0.8f /*volume*/);

    // 3. Play looping background music
    Audio::playMusic("assets/bgm.wav", 0.5f, true /*loop*/);

    // In game loop:
    while (window.isOpen()) {
        window.pollEvents();
        Audio::update(); // Updates music looping stream

        if (Input::isKeyJustPressed(Key::Space)) {
            Audio::play("assets/jump.wav");
        }
    }

    // 4. Cleanup
    Audio::shutdown();
    return 0;
}
```

---

## 2. Audio API Reference

| Method | Description |
| :--- | :--- |
| `Audio::init()` | Initializes the default audio playback device |
| `Audio::shutdown()` | Closes the audio device and cleans up streams |
| `Audio::play(filePath, volume = 1.0f)` | Plays a sound effect file once |
| `Audio::play(sound, volume = 1.0f)` | Plays a preloaded `Sound` object |
| `Audio::playMusic(filePath, volume, loop = true)` | Starts streaming background music |
| `Audio::stopMusic()` | Stops background music playback |
| `Audio::setMasterVolume(volume)` | Sets global master volume (`0.0f` to `1.0f`) |
| `Audio::getMasterVolume()` | Returns current master volume |
| `Audio::update()` | Keeps background music streaming and loops queued |

---

## 3. Preloaded Sound Objects (`Sound`)

For low-latency repeating sound effects:

```cpp
Sound jumpSound("assets/jump.wav");

// Play instantly anytime
Audio::play(jumpSound, 1.0f);
```
