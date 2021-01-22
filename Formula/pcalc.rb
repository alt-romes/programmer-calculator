# Documentation: https://docs.brew.sh/Formula-Cookbook
#                https://rubydoc.brew.sh/Formula
class Pcalc < Formula
  desc "CLI Programming Calculator"
  homepage ""
  url "https://github.com/alt-romes/programmer-calculator/archive/v1.1.tar.gz"
  sha256 "78b40da3abb4dc6619cbf0e09e03ab7ecda89cad6184945c143968830c145f3c"
  license "GNU General Public License v3.0"

  depends_on "cmake" => :build
  depends_on "ncurses"

  def install
    system "make", "install"
    bin.install "pcalc"
  end

  test do
    system "true"
  end
end
