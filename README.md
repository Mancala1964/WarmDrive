# Warm Drive

A simple saturation plugin with four knobs, built with [JUCE](https://juce.com).

| Knob   | What it does |
|--------|--------------|
| Drive  | How hard the sound is pushed into saturation (0 to 36 dB). The volume is compensated automatically, so this changes the character more than the loudness. |
| Tone   | 0% = dark/warm, 100% = bright. |
| Mix    | 0% = clean original, 100% = fully saturated. |
| Output | Final volume (-24 to +12 dB). |

Double-click any knob to reset it.

You build it on Windows, GitHub builds the Mac **Audio Unit** for GarageBand, and you install that on the Mac.

---

## 1. Try it on Windows (optional, quick check)

Open **Developer PowerShell for VS** (or any terminal), go to this folder and run:

```
cmake -B build
cmake --build build --config Release --target WarmDrive_Standalone
```

The first run downloads JUCE, so it takes a few minutes. Then open:

```
build\WarmDrive_artefacts\Release\Standalone\Warm Drive.exe
```

Click **Options > Audio/MIDI Settings**, pick your mic or audio interface as the input, and untick
"Mute audio input". **Use headphones** so you don't get feedback.

> You can also open this folder in Visual Studio (**File > Open > Folder**). It detects CMake on its own.
> Choose `Warm Drive.exe` (Standalone) as the startup item and press F5.

## 2. Put it on GitHub (this builds the Mac version)

In **GitHub Desktop**:

1. **File > Add local repository** and choose this `WarmDrive` folder.
2. It will say this isn't a Git repository yet. Click **create a repository**, then **Create repository**.
3. Click **Publish repository**.
   - A public repo builds for free.
   - A private repo also works, but GitHub's free monthly build allowance is limited, and Mac builds use it up about 10× faster than Linux ones.
4. On github.com, open your repo and go to the **Actions** tab. A build named **"Build Mac plugin"** starts on its own and takes about 5–10 minutes.
5. When it shows a green check, click into it. At the bottom under **Artifacts**, download **WarmDrive-mac.zip**.

From now on, every time you **Commit** and **Push** in GitHub Desktop, a fresh Mac build is made.

## 3. Install it on the Mac

1. Copy `WarmDrive-mac.zip` to the Mac (AirDrop, USB stick, or just download it on the Mac).
2. Double-click the zip to unpack it.
3. Follow `HOW-TO-INSTALL.txt` inside it. In short: in Terminal, type `bash `, drag in `install.sh`, and press Return.
4. Quit and reopen GarageBand. Then go to **Smart Controls (B) > Plug-ins > Audio Units > Ortiz > Warm Drive**.

## Project layout

```
CMakeLists.txt              build settings (plugin name, codes, formats)
Source/PluginProcessor.*    the sound processing (DSP)
Source/PluginEditor.*       the window with the knobs
.github/workflows/          the Mac build recipe GitHub runs
mac/                        installer script + instructions packed into the zip
```

How the sound is processed: input → 2× oversampling → Drive gain → `tanh` soft-clip → back to normal rate
→ Tone (low-pass filter) → level compensation → Mix with the clean signal → Output.

## Troubleshooting

- **The GitHub build failed at "Validate with Apple's auval".** The zip was still uploaded, so you can test it in GarageBand anyway. Send me the log and we'll fix it.
- **GarageBand doesn't list it.** Check **GarageBand > Settings > Audio/MIDI > Enable Audio Units**. Then restart the Mac once.
- **CMake says the version is too old.** This project needs CMake 3.22 or newer. Run `cmake --version` to check.
