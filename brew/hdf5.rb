class Hdf5 < Formula
  homepage "https://www.hdfgroup.org/HDF5/"
  url "https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8/hdf5-1.8.19/src/hdf5-1.8.19.tar.bz2"
  sha256 "59c03816105d57990329537ad1049ba22c2b8afe1890085f0c022b75f1727238"
  def install
    system "./configure", "--prefix=#{prefix}"
    system "make", "install"
  end
end
