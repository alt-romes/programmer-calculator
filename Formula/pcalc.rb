# Documentation: https://docs.brew.sh/Formula-Cookbook
#                https://rubydoc.brew.sh/Formula
class Pcalc < Formula
  desc "CLI Programming Calculator"
  homepage ""
  url "https://github.com/alt-romes/programmer-calculator/archive/v1.5.tar.gz"
  sha256 "65bb8f21ad256e8e65827ca00e8d51f5d1f0395a08f7cc44747bccfc10bdcc3e"
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
