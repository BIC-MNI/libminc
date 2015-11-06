#!/bin/bash
if [ "${TRAVIS_OS_NAME}" = "osx" ]; then
    brew install ./brew/netcdf.rb
    brew install ./brew/hdf5.rb
fi

