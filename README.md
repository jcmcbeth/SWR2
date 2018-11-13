# Star Wars Reality 2.0 MUD
This is the original version of SWR 2.0 that you could find on Geocities.

You can find the files here: http://www.oocities.org/gendi_uk/

## How to build
Using Ubuntu Server 18.10 use the following commands:
```bash
sudo apt-get install csh make gcc

git clone https://github.com/jcmcbeth/swr2.git
cd swr2/swr-2.0/src

make all

cd ../..
chmod +x run-swr

./run-swr &
```

You are also going to want to extract the initial data.zip file or the MUD will fail to start.

Now you can telnet to the mud on port 9999.
