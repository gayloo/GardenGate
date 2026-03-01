# Running dedicated server on Linux

### Prerequisites

Warning: Run everything here within a tmux session if you'd like it to continue running once you log out of ssh

Ensure the following dependencies are installed on your host:

* podman
* tmux
* git
* text editor (e.g. vim)

### Generating `auth.toml`

Download the pre-release version of maxima from [here](https://github.com/silkker/MARNE/releases/tag/0.1)

After logging in, locate the generated `auth.toml` file:

- Windows:
  `%APPDATA%\Roaming\ArmchairDevelopers\Maxima\data`

* Linux:
  `$HOME/.local/share/maxima`

Copy the contents of `auth.toml` into the same `auth.toml` inside the repository.

### Podman Setup

On the machine that will host the dedicated server, execute the following commands individually:

```sh
mkdir -p $HOME/Games
mkdir -p $HOME/.local/share/Steam/compatibilitytools.d
git clone https://github.com/nocss42/GardenGate
cd GardenGate/Container
vim auth.toml   # Paste the contents from your main machine
vim tmux.conf   # Optional (uncomment from Containerfile if used)
podman build --no-cache -t gardengate:latest .
```

### Running the Container

Run the container with:

```sh
podman run --replace -it \
  --name gardengate \
  -v $HOME/Games:/opt/games \
  -p 25200:25200/tcp \
  -p 25200:25200/udp \
  -p 25200:25201/tcp \
  -p 25200:25201/udp \
  --security-opt seccomp=unconfined \
  gardengate:latest
```

Type exit once it gets into a shell.

Start the container by running

```sh
podman start -ai gardengate
```

### Game Directory Setup

Create the dir for the game you're going to install:

e.g.

```sh
sudo chmod 777 /opt/games
mkdir -p /opt/games/gw2
```

Install the game via maxima:

```sh
wlheadless-run -c cage -- $HOME/maxima.sh
```

(Press `Ctrl+C` once completed.)

### Registry Configuration

Import required registry files:

```sh
umu-run $HOME/.local/share/maxima/wine/prefix/drive_c/windows/syswow64/regedit.exe \
$HOME/dll_overrides.reg
```

```sh
umu-run $HOME/.local/share/maxima/wine/prefix/drive_c/windows/syswow64/regedit.exe \
$HOME/gw2.reg
```

### Patch the game

Patch the game by running;

```sh
curl -L https://github.com/nocss42/GardenGate/releases/latest/download/GardenGate.zip \
     -o /opt/games/GardenGate.zip
unzip /opt/games/GardenGate.zip -d /opt/games
umu-run /opt/games/GardenGate_Launcher.exe --patch gw2 /opt/games/gw2/
```

### Server Configuration

Edit server settings as you wish:

```sh
vim /opt/games/gw2/ServerSettings.txt
```

### Maxima game slugs
```
plants-vs-zombies-garden-warfare
plants-vs-zombies-garden-warfare-2
pvz-battle-for-neighborville
```

Launch the server;

```sh
wlheadless-run -c cage -- maxima-cli launch \
  --game-path /opt/games/gw2/GW2.Main_Win64_Retail.exe \
  --game-args=-server plants-vs-zombies-garden-warfare-2
```

### Installing mods

Get a VPS with RDP setup or use waypipe (Frosty CLI is still way too WIP to be of use)

Download frosty inside the container like so (these are here for reference don't actually run them)

```sh
# GW1
wget -O /opt/games/GW1_FrostyModManager.zip https://github.com/Twig6943/FrostyToolsuiteGW1Linux/releases/latest/download/PVZGW_FrostyModManager.zip
unzip /opt/games/GW1_FrostyModManager.zip -d /opt/games
# GW2
wget -O /opt/games/GW2_FrostyModManager.tar.gz https://www.nexusmods.com/masseffectandromeda/mods/1190?tab=files&file_id=6904
unzip /opt/games/GW2_FrostyModManager.zip -d /opt/games
# BFN
wget -O /opt/games/BFN_FrostyModManager.zip https://github.com/Twig6943/FrostyToolsuiteBFNLinux/releases/latest/download/FrostyModManager.zip
unzip /opt/games/BFN_FrostyModManager.zip -d /opt/games
```

Outside of podman, install umu and ge-proton for your distro like so (these commands are here as reference please go ahead and grab the latest version of everything);

```sh
export PROTON_ENABLE_WAYLAND=1 # If you're on a wayland session
export PROTONPATH=$HOME/.local/share/Steam/compatibilitytools.d/GE-Proton10-32

wget -O $HOME/ge-proton.tar.gz https://github.com/GloriousEggroll/proton-ge-custom/releases/download/GE-Proton10-32/GE-Proton10-32.tar.gz
tar -xzf $HOME/ge-proton.tar.gz -C "$HOME/.local/share/Steam/compatibilitytools.d"

sudo dpkg --add-architecture i386 # needed for debian-based

wget -O $HOME/umu.deb https://github.com/Open-Wine-Components/umu-launcher/releases/download/1.3.0/python3-umu-launcher_1.3.0-1_amd64_debian-13.deb
sudo dpkg -i $HOME/umu.deb

sudo chmod -R 777 $HOME/Games
umu-run $HOME/Games/GW2_FrostyModManager/FrostyModManager.exe
```

To start modded server, inside the container run;
```sh
wlheadless-run -c cage -- \
  maxima-cli launch \
  --game-path "/opt/games/gw2/GW2.Main_Win64_Retail.exe" \
  --game-args="-server" \
  --game-args="-dataPath" \
  --game-args="ModData/Default" \
  plants-vs-zombies-garden-warfare-2
```





