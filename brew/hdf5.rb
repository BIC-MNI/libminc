class Hdf5 < Formula
  homepage "https://www.hdfgroup.org/HDF5/"
  url "https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8/hdf5-1.8.19/src/hdf5-1.8.19.tar.bz2"
  sha256 "8755d7290c4274de5e897840b2bcd15cffe7b5cbcac4a7bc0c430b83d7f0944b"
  def install
    system "./configure", "--prefix=#{prefix}"
    system "make", "install"
  end
end
