# Star Wars Reality 2.0 MUD
This is the original version of SWR 2.0 that you could find on Geocities.

You can find the files here: http://www.oocities.org/gendi_uk/

## How to build
On ubuntu Ubuntu Server 18.10 use the following commands:
git clone https://github.com/jcmcbeth/SWR2.git
cd swr2/swr-2.0/src

sudo apt-get install csh
sudo apt-get install make
sudo apt-get install gcc

make all

cd ../..
sudo chmod +x run-swr

./run-swr &

Now you can telnet to the mud on port 9999.

Note: For the original source I had to uncomment the lcrypt flag from the Makefile.
