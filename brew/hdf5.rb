class Hdf5 < Formula
  homepage "https://www.hdfgroup.org/HDF5/"
  url "https://www.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8.15-patch1/src/hdf5-1.8.15-patch1.tar.bz2"
  md5 "3c0d7a8c38d1abc7b40fc12c1d5f2bb8"
  def install
    system "./configure", "--prefix=#{prefix}"
    system "make", "install"
  end
end
