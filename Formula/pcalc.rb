# Documentation: https://docs.brew.sh/Formula-Cookbook
#                https://rubydoc.brew.sh/Formula
class Pcalc < Formula
  desc "CLI Programming Calculator"
  homepage ""
  url "https://github.com/alt-romes/programmer-calculator/archive/v1.4.tar.gz"
  sha256 "8fb21d0902cda676944f178a890c8f7302497104202a4e942913871ffb45098f"
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
