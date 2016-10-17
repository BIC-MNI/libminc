class Hdf5 < Formula
  homepage "https://www.hdfgroup.org/HDF5/"
  url "https://www.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8.15-patch1/src/hdf5-1.8.15-patch1.tar.bz2"
  sha256 "a5afc630c4443547fff15e9637b5b10404adbed4c00206d89517d32d6668fb32"
  def install
    system "./configure", "--prefix=#{prefix}"
    system "make", "install"
  end
end
