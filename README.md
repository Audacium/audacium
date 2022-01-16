[![Audacium](.github/audacium_logo.png)](http://audacium.xyz)
<br/>

# FFmpeg 5.0 experimental branch
To be able to run Audacium, you'll need to link the .lib files (for Windows at least) with Visual Studio (project properties -> Linker -> Input -> Additional Dependencies), then copy all .dll files into the directory where Audacium.exe will be located when building.

## Searching for contributors!
I'm currently searching for contributors as I can't keep up the project alone. If you're interested to become one, make an issue!

## Goals of this project
The goals **Audacium** is trying to achieve are pretty simple:
- Restoring Audacity to what it was before, with no crash reporting (GH Issues exist for that), or very simple update checking (only at startup, not periodically).
- Adding new features people want, listening closely to the community.
- Make the code base easier to work with, less confusing for a newbie.

----------------

[**Audacium**](http://audacium.xyz) is an easy-to-use, multi-track audio editor and recorder for Windows, macOS, GNU/Linux and other operating systems. Developed by a group of volunteers as open source.

- **Recording** from any real, or virtual audio device that is available to the host system.
- **Export / Import** a wide range of audio formats, extendible with FFmpeg.
- **High quality** using 32-bit float audio processing.
- **Plug-ins** Support for multiple audio plug-in formats, including VST, LV2, AU.
- **Macros** for chaining commands and batch processing.
- **Scripting** in Python, Perl, or any language that supports named pipes.
- **Nyquist** Very powerful built-in scripting language that may also be used to create plug-ins.
- **Editing** multi-track editing with sample accuracy and arbitrary sample rates.
- **Accessibility** for VI users.
- **Analysis and visualization** tools to analyze audio, or other signal data.

## Getting Started

For end users, the latest Windows and macOS release version of Audacium is available [here](https://github.com/SartoxSoftware/audacium/releases/latest).
There is also an [AUR package](https://aur.archlinux.org/packages/audacium/) for Arch Linux users.
Help with using Audacium is available from the [Matrix room](https://matrix.to/#/#audacium:envs.net) or via the [Discord server](https://discord.gg/ZH5234Abhb) (the two are bridged together).

Build instructions are available [here](https://github.com/SartoxOnlyGNU/audacium/wiki/Building).

More information is available from the [wiki](https://github.com/SartoxOnlyGNU/audacium/wiki).

## How to contribute
Submit a pull request, then we'll carefully review it and merge it if possible.

## Donations
You can donate to me via **Litecoin** using this wallet: LfA7A3wVdGysc4QXpsUvYaVUYvhcZnVE7j
