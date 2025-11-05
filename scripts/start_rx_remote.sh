#!/bin/bash

SRC_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && cd ../ && pwd )

rm -rvf $HOME/.config/qradiolink/*
mkdir -p $HOME/.config/qradiolink
cp -v $SRC_DIR/scripts/rx_jupiter_config.cfg $HOME/.config/qradiolink/qradiolink.cfg

$SRC_DIR/build/qradiolink