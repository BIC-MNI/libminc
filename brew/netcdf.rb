class Netcdf < Formula
  homepage "http://www.unidata.ucar.edu/software/netcdf/"
  url "ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4.3.3.1.tar.gz"
  sha256 "bdde3d8b0e48eed2948ead65f82c5cfb7590313bc32c4cf6c6546e4cea47ba19"
  def install
    system "./configure", "--prefix=#{prefix}", "--disable-netcdf-4"
    system "make", "install"
  end
end
