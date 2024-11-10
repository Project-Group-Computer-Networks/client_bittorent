# OurTorrent

OurTorrent is our attempt at developing a bittorrent client, which enables users to download files over internet in a P2P manner.

## Installation and Build :hammer_and_wrench:

* Clone this repository on an Ubuntu machine and move to the working directory
```
    $ git clone https://github.com/Project-Group-Computer-Networks/client_bittorent.git
    $ cd ourtorrent_client
```

* Donwload all the pre-requisites present in requirements.txt 
```
    $ pip install -r requirements.txt
```

## Run :computer:

* Change directory to src and run the help command
```
    $ cd src
    $ python3 main.py --help
```

* To test the code, run the client with a torrent file present in examples folder
```
    $ python3 main.py -d ../ ../examples/ubuntu.torrent
```
