# Documentation: https://docs.brew.sh/Formula-Cookbook
#                https://rubydoc.brew.sh/Formula
class ProgrammerCalculator < Formula
  desc "CLI Programming Calculator"
  homepage ""
  url "https://github.com/alt-romes/programmer-calculator/archive/v1.0.tar.gz"
  sha256 "399524be160bf23251a170c949262204d650561b1e58c47af239284b3e3ae1da"
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
