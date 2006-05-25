#!/bin/bash
export LD_LIBRARY_PATH=..:../../externals:$LD_LIBRARY_PATH

./tests
