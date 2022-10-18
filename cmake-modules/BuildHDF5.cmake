macro(build_hdf5 install_prefix)

ExternalProject_Add(HDF5
        SOURCE_DIR HDF5
        URL "https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8/hdf5-1.8.7/src/hdf5-1.8.7.tar.gz"
        URL_HASH SHA256=0adbc0eacafecabeb3ad2ebe64f4ef7fdad4f33e12b5bbf6630a8c37a72db1a9
        BUILD_IN_SOURCE 1
        INSTALL_DIR     "${install_prefix}"
        BUILD_COMMAND   make 
        INSTALL_COMMAND make install 
        CONFIGURE_COMMAND ./configure --prefix=${install_prefix}  --with-pic --disable-shared --disable-cxx --disable-f77 --disable-f90 --disable-examples --disable-hl --disable-docs
      )

SET(HDF5_INCLUDE_DIR ${install_prefix}/include )
SET(HDF5_LIBRARY  ${install_prefix}/lib/libhdf5.a )


endmacro(build_hdf5)