#!/bin/sh

PATH=$PATH:$HOME/.nimble/bin LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64 albinos_editor --cli --load-config="configKey.txt" 2> openConfig_stderr
