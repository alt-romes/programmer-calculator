# Documentation: https://docs.brew.sh/Formula-Cookbook
#                https://rubydoc.brew.sh/Formula
class Pcalc < Formula
  desc "CLI Programming Calculator"
  homepage ""
  url "https://github.com/alt-romes/programmer-calculator/archive/v2.0.tar.gz"
  sha256 "8380ea0a0cf156c0c97167b776307f67b41acebe58c141634ede6e6f088f55dc"
  license "GNU General Public License v3.0"

  depends_on "cmake" => :build
  depends_on "ncurses"

  def install
    system "make"
    bin.install "pcalc"
  end

  test do
    system "true"
  end
end
