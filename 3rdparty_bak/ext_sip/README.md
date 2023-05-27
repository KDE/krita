Notes on SIP

Building

export BUILDROOT=$HOME/dev
python3 configure.py -b $BUILDROOT/deps/bin -d $BUILDROOT/deps/sip -e $BUILDROOT/deps/include  --sipdir $BUILDROOT/deps/sip --target-py-version 3.4

